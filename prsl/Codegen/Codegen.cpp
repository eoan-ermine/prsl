#include "prsl/Codegen/Codegen.hpp"
#include "prsl/AST/NodeTypes.hpp"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Verifier.h"
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/FileSystem.h>

namespace prsl::Codegen {

Codegen::Codegen(ErrorReporter &eReporter)
    : eReporter(eReporter), context(std::make_unique<LLVMContext>()),
      module(std::make_unique<Module>("Prsl", *context)),
      builder(std::make_unique<IRBuilder<>>(*context)),
      envManager(this->eReporter), intType(llvm::Type::getInt32Ty(*context)) {}

bool Codegen::dump(std::string_view filename) const {
  std::error_code ec;
  auto fileStream =
      llvm::raw_fd_ostream(filename, ec, llvm::sys::fs::OpenFlags::OF_None);
  if (ec)
    return false;

  module->print(fileStream, nullptr);
  return true;
}

Value *Codegen::visitLiteralExpr(const LiteralExprPtr &expr) {
  return ConstantInt::get(intType, expr->literalVal);
}

Value *Codegen::visitGroupingExpr(const GroupingExprPtr &expr) {
  return visitExpr(expr->expression);
}

Value *Codegen::visitVarExpr(const VarExprPtr &expr) {
  AllocaInst *V = getAllocVar(expr->ident);
  return builder->CreateLoad(intType, V, expr->ident.getLexeme());
}

Value *Codegen::visitInputExpr(const InputExprPtr &expr) {
  BasicBlock *insertBB = builder->GetInsertBlock();
  Function *func_scanf = module->getFunction("scanf");

  if (!func_scanf) {
    std::vector<llvm::Type *> ints(0, intType);
    FunctionType *funcType = FunctionType::get(intType, ints, false);
    func_scanf = Function::Create(funcType, Function::ExternalLinkage, "scanf",
                                  module.get());
    func_scanf->setCallingConv(CallingConv::C);
  }

  Value *str = builder->CreateGlobalStringPtr("%d");
  auto *temp_var = allocVar("inputtemp");
  std::vector<Value *> call_params;
  call_params.push_back(str);
  call_params.push_back(temp_var);

  CallInst *call =
      llvm::CallInst::Create(func_scanf, call_params, "calltmp", insertBB);
  return builder->CreateLoad(intType, temp_var, "inputres");
}

Value *Codegen::visitAssignmentExpr(const AssignmentExprPtr &expr) {
  Value *value = visitExpr(expr->initializer);
  AllocaInst *varInst = getOrCreateAllocVar(expr->varName);
  builder->CreateStore(value, varInst);
  return value;
}

Value *Codegen::visitUnaryExpr(const UnaryExprPtr &expr) {
  Value *value = visitExpr(expr->expression);

  switch (expr->op.getType()) {
  case Token::Type::MINUS:
    return builder->CreateNSWNeg(value);
  default:
    break;
  }

  throw Errors::reportRuntimeError(eReporter, expr->op,
                                   "Illegal unary expression");
}

Value *Codegen::visitBinaryExpr(const BinaryExprPtr &expr) {
  Value *lhs = visitExpr(expr->lhsExpression);
  Value *rhs = visitExpr(expr->rhsExpression);
  if (!lhs || !rhs)
    return nullptr;

  switch (expr->op.getType()) {
  case Token::Type::PLUS:
    return builder->CreateAdd(lhs, rhs, "addtmp");
  case Token::Type::MINUS:
    return builder->CreateSub(lhs, rhs, "subtmp");
  case Token::Type::STAR:
    return builder->CreateMul(lhs, rhs, "multmp");
  case Token::Type::SLASH:
    return builder->CreateSDiv(lhs, rhs, "divtmp");
  case Token::Type::NOT_EQUAL:
    return builder->CreateICmpNE(lhs, rhs, "netmp");
  case Token::Type::EQUAL_EQUAL:
    return builder->CreateICmpEQ(lhs, rhs, "etmp");
  case Token::Type::LESS:
    return builder->CreateICmpSLT(lhs, rhs, "ltmp");
  case Token::Type::LESS_EQUAL:
    return builder->CreateICmpSLE(lhs, rhs, "letmp");
  case Token::Type::GREATER:
    return builder->CreateICmpSGT(lhs, rhs, "gtmp");
  case Token::Type::GREATER_EQUAL:
    return builder->CreateICmpSGE(lhs, rhs, "getmp");
  default:
    break;
  }

  throw reportRuntimeError(eReporter, expr->op,
                           "Illegal operator in expression");
}

Value *Codegen::postfixExpr(const Token &op, Value *obj, Value *res) {
  Value *constOne = ConstantInt::get(intType, 1);

  Value *value;
  switch (op.getType()) {
  case Token::Type::PLUS_PLUS:
    value = builder->CreateAdd(obj, constOne, "inctmp");
    break;
  case Token::Type::MINUS_MINUS:
    value = builder->CreateSub(obj, constOne, "dectmp");
    break;
  default:
    throw reportRuntimeError(eReporter, op, "Illegal operator in expression");
  }

  return builder->CreateStore(value, res);
}

Value *Codegen::visitPostfixExpr(const PostfixExprPtr &expr) {
  Value *obj = visitExpr(expr->expression);
  if (std::holds_alternative<VarExprPtr>(expr->expression)) {
    const auto &varExpr = std::get<VarExprPtr>(expr->expression);
    postfixExpr(expr->op, obj, getAllocVar(varExpr->ident));
  }
  return obj;
}

Value *Codegen::evaluateScope(const ScopeExprPtr &stmt) {
  for (const auto &stmt : stmt->statements) {
    visitStmt(stmt);
    if (returnStack.size()) {
      auto [returnValue, isFunction] = std::move(returnStack.top());
      returnStack.pop();
      return returnValue;
    }
  }
  return nullptr;
}

Value *Codegen::visitScopeExpr(const ScopeExprPtr &stmt) {
  Value *res;
  envManager.withNewEnviron([&] { res = evaluateScope(stmt); });
  return res;
}

Value *Codegen::visitFuncExpr(const FuncExprPtr &expr) {
  auto *previousBB = builder->GetInsertBlock();

  std::vector<llvm::Type *> argTypes(expr->parameters.size(), intType);
  FunctionType *ftype = FunctionType::get(intType, argTypes, false);
  Function *func = Function::Create(
      ftype, Function::ExternalLinkage,
      expr->name ? expr->name->getLexeme() : "func", module.get());

  // Create a new basic block to start insertion into.
  BasicBlock *BB = BasicBlock::Create(*context, "entry", func);
  builder->SetInsertPoint(BB);

  auto funcEnv = std::make_shared<Types::Environment<Value *>>(nullptr);

  if (expr->name)
    functionsManager.set(expr->name->getLexeme(), func);

  Value *res = nullptr;
  envManager.withNewEnviron(funcEnv, [&]() {
    auto argsIt = func->args().begin();
    auto paramsIt = expr->parameters.begin();
    for (; argsIt != func->args().end() && paramsIt != expr->parameters.end();
         argsIt++, paramsIt++) {
      auto *allocaInst = allocVar(paramsIt->getLexeme());
      envManager.define(*paramsIt, allocaInst);
      builder->CreateStore(argsIt, allocaInst);
    }

    auto scopeRes = evaluateScope(std::get<ScopeExprPtr>(expr->body));
  });

  verifyFunction(*func);
  builder->SetInsertPoint(previousBB);
  return func;
}

Value *Codegen::visitCallExpr(const CallExprPtr &expr) {
  Function *func = getFunction(expr->ident);
  if (functionsManager.contains(expr->ident.getLexeme()))
    func = functionsManager.get(expr->ident.getLexeme());

  if (!func)
    throw reportRuntimeError(eReporter, expr->ident, "Not a function");

  if (func->arg_size() != expr->arguments.size()) {
    throw reportRuntimeError(eReporter, expr->ident,
                             "Wrong number of arguments");
  }

  std::vector<Value *> args;
  for (const auto &arg : expr->arguments) {
    args.push_back(visitExpr(arg));
  }

  return builder->CreateCall(func, args, "calltmp");
}

Value *Codegen::visitVarStmt(const VarStmtPtr &stmt) {
  Value *value = visitExpr(stmt->initializer);
  if (isa<Function>(value)) {
    envManager.define(stmt->varName, value);
  } else {
    AllocaInst *varInst = getOrCreateAllocVar(stmt->varName);
    builder->CreateStore(value, varInst);
  }
  return value;
};

Value *Codegen::visitIfStmt(const IfStmtPtr &stmt) {
  Value *conditionV = visitExpr(stmt->condition);
  conditionV = builder->CreateICmpNE(
      conditionV, ConstantInt::get(llvm::Type::getInt1Ty(*context), 0),
      "condtmp");

  BasicBlock *insertBB = builder->GetInsertBlock();
  Function *function = insertBB->getParent();

  BasicBlock *thenBB = BasicBlock::Create(*context, "then", function);
  BasicBlock *elseBB;
  BasicBlock *mergeBB = BasicBlock::Create(*context, "ifcont");

  if (stmt->elseBranch) {
    elseBB = BasicBlock::Create(*context, "else");
    builder->CreateCondBr(conditionV, thenBB, elseBB);
  } else {
    builder->CreateCondBr(conditionV, thenBB, mergeBB);
  }

  builder->SetInsertPoint(thenBB);
  visitStmt(stmt->thenBranch);
  if (returnStack.size()) {
    returnStack.pop();
  } else {
    builder->CreateBr(mergeBB);
  }

  thenBB = builder->GetInsertBlock();

  if (stmt->elseBranch) {
    function->insert(function->end(), elseBB);
    builder->SetInsertPoint(elseBB);
    visitStmt(*stmt->elseBranch);
    if (returnStack.size()) {
      returnStack.pop();
    } else {
      builder->CreateBr(mergeBB);
    }

    elseBB = builder->GetInsertBlock();
  }

  function->insert(function->end(), mergeBB);
  builder->SetInsertPoint(mergeBB);
  return nullptr;
}

Value *Codegen::visitWhileStmt(const WhileStmtPtr &stmt) {
  Function *function = builder->GetInsertBlock()->getParent();

  BasicBlock *conditionBB = BasicBlock::Create(*context, "condition", function);
  BasicBlock *loopBB = BasicBlock::Create(*context, "loop", function);
  BasicBlock *afterBB = BasicBlock::Create(*context, "afterloop", function);

  builder->CreateBr(conditionBB);
  builder->SetInsertPoint(conditionBB);
  Value *conditionV = visitExpr(stmt->condition);
  conditionV = builder->CreateICmpNE(
      conditionV, ConstantInt::get(llvm::Type::getInt1Ty(*context), 0),
      "loopcondition");
  builder->CreateCondBr(conditionV, loopBB, afterBB);

  builder->SetInsertPoint(loopBB);
  visitStmt(stmt->body);
  if (returnStack.size()) {
    returnStack.pop();
  } else {
    builder->CreateBr(conditionBB);
  }

  builder->SetInsertPoint(afterBB);
  return nullptr;
}

Value *Codegen::visitPrintStmt(const PrintStmtPtr &stmt) {
  Value *val = visitExpr(stmt->value);
  BasicBlock *insertBB = builder->GetInsertBlock();
  Function *func_printf = module->getFunction("printf");

  if (!func_printf) {
    std::vector<llvm::Type *> ints(1, intType);
    FunctionType *funcType = FunctionType::get(intType, ints, false);
    func_printf = Function::Create(funcType, Function::ExternalLinkage,
                                   "printf", module.get());
    func_printf->setCallingConv(CallingConv::C);
  }

  Value *str = builder->CreateGlobalStringPtr("%d\n");
  std::vector<llvm::Value *> call_params;
  call_params.push_back(str);
  call_params.push_back(val);
  CallInst::Create(func_printf, call_params, "calltmp", insertBB);

  return val;
}

Value *Codegen::visitExprStmt(const ExprStmtPtr &stmt) {
  visitExpr(stmt->expression);
  return nullptr;
}

Value *Codegen::visitFunctionStmt(const FunctionStmtPtr &stmt) {
  FunctionType *FT = FunctionType::get(llvm::Type::getInt32Ty(*context),
                                       std::vector<llvm::Type *>{}, false);
  Function *F =
      Function::Create(FT, Function::ExternalLinkage, "main", module.get());
  BasicBlock *BB = BasicBlock::Create(*context, "", F);
  builder->SetInsertPoint(BB);

  for (const auto &stmt : stmt->body) {
    visitStmt(stmt);
  }

  builder->CreateRet(ConstantInt::get(intType, 0));
  return F;
}

Value *Codegen::visitBlockStmt(const BlockStmtPtr &stmt) {
  envManager.withNewEnviron([&] {
    for (const auto &stmt : stmt->statements) {
      visitStmt(stmt);
    }
  });
  return nullptr;
}

Value *Codegen::visitReturnStmt(const ReturnStmtPtr &stmt) {
  auto returnValue = visitExpr(stmt->retValue);
  if (stmt->isFunction) {
    builder->CreateRet(returnValue);
  }
  returnStack.push({returnValue, stmt->isFunction});
  return nullptr;
}

Value *Codegen::visitNullStmt(const NullStmtPtr &stmt) { return nullptr; }

AllocaInst *Codegen::allocVar(std::string_view name) {
  BasicBlock *insertBB = builder->GetInsertBlock();
  Function *func = insertBB->getParent();
  builder->SetInsertPoint(&func->getEntryBlock(),
                          func->getEntryBlock().begin());
  AllocaInst *inst = builder->CreateAlloca(intType, 0, name);
  builder->SetInsertPoint(insertBB);
  return inst;
}

AllocaInst *Codegen::getAllocVar(const Token &ident) {
  if (envManager.contains(ident))
    return cast<AllocaInst>(envManager.get(ident));
  return nullptr;
}

Function *Codegen::getFunction(const Token &ident) {
  if (envManager.contains(ident) && isa<Function>(envManager.get(ident)))
    return cast<Function>(envManager.get(ident));
  return nullptr;
}

AllocaInst *Codegen::getOrCreateAllocVar(const Token &variable) {
  if (auto res = getAllocVar(variable))
    return res;
  auto inst = allocVar(variable.getLexeme());
  envManager.define(variable, inst);
  return inst;
}

} // namespace prsl::Codegen