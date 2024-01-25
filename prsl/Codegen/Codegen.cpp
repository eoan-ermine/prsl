#include "Codegen.hpp"

namespace prsl::Codegen {

Codegen::Codegen(ErrorReporter &eReporter)
    : eReporter(eReporter), context(std::make_unique<LLVMContext>()),
      module(std::make_unique<Module>("Prsl", *context)),
      builder(std::make_unique<IRBuilder<>>(*context)),
      envManager(this->eReporter), intType(llvm::Type::getInt32Ty(*context)) {
  FunctionType *FT = FunctionType::get(llvm::Type::getInt32Ty(*context),
                                       std::vector<llvm::Type *>{}, false);
  Function *F =
      Function::Create(FT, Function::ExternalLinkage, "main", module.get());
  BasicBlock *BB = BasicBlock::Create(*context, "", F);
  builder->SetInsertPoint(BB);
}

Value *Codegen::codegenExpr(const ExprPtrVariant &expr) {
  switch (expr.index()) {
  case 0:
    return codegenLiteralExpr(std::get<0>(expr));
  case 1:
    return codegenGroupingExpr(std::get<1>(expr));
  case 2:
    return codegenVarExpr(std::get<2>(expr));
  case 3:
    return codegenInputExpr(std::get<3>(expr));
  case 4:
    return codegenAssignmentExpr(std::get<4>(expr));
  case 5:
    return codegenUnaryExpr(std::get<5>(expr));
  case 6:
    return codegenBinaryExpr(std::get<6>(expr));
  case 7:
    return codegenPostfixExpr(std::get<7>(expr));
  default:
    std::unreachable();
  }
}

Value *Codegen::codegenStmt(const StmtPtrVariant &stmt) {
  switch (stmt.index()) {
  case 0:
    return codegenVarStmt(std::get<0>(stmt));
  case 1:
    return codegenIfStmt(std::get<1>(stmt));
  case 2:
    return codegenBlockStmt(std::get<2>(stmt));
  case 3:
    return codegenWhileStmt(std::get<3>(stmt));
  case 4:
    return codegenPrintStmt(std::get<4>(stmt));
  case 5:
    return codegenExprStmt(std::get<5>(stmt));
  default:
    std::unreachable();
  }
}

void Codegen::executeStmts(const std::vector<StmtPtrVariant> &stmts) {
  for (const auto &stmt : stmts) {
    try {
      codegenStmt(stmt);
    } catch (const Errors::RuntimeError &e) {
      eReporter.printToErr();
    }
  }
}

void Codegen::dump(std::string_view filename) {
  builder->CreateRet(ConstantInt::get(intType, 0));

  std::error_code ec;
  auto fileStream =
      llvm::raw_fd_ostream(filename, ec, llvm::sys::fs::OpenFlags::OF_None);

  module->print(fileStream, nullptr);
}

Value *Codegen::codegenLiteralExpr(const LiteralExprPtr &expr) {
  return ConstantInt::get(intType, expr->literalVal);
}

Value *Codegen::codegenGroupingExpr(const GroupingExprPtr &expr) {
  return codegenExpr(expr->expression);
}

Value *Codegen::codegenVarExpr(const VarExprPtr &expr) {
  AllocaInst *V = envManager.get(expr->ident);
  return builder->CreateLoad(intType, V, expr->ident.getLexeme());
}

Value *Codegen::codegenInputExpr(const InputExprPtr &expr) {
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

Value *Codegen::codegenAssignmentExpr(const AssignmentExprPtr &expr) {
  Value *value = codegenExpr(expr->initializer);
  AllocaInst *varInst = getOrCreateAllocVar(expr->varName);
  builder->CreateStore(value, varInst);
  return value;
}

Value *Codegen::codegenUnaryExpr(const UnaryExprPtr &expr) {
  Value *value = codegenExpr(expr->expression);

  switch (expr->op.getType()) {
  case Token::Type::MINUS:
    return builder->CreateNSWNeg(value);
  default:
    break;
  }

  throw Errors::reportRuntimeError(eReporter, expr->op,
                                   "Illegal unary expression");
}

Value *Codegen::codegenBinaryExpr(const BinaryExprPtr &expr) {
  Value *lhs = codegenExpr(expr->lhsExpression);
  Value *rhs = codegenExpr(expr->rhsExpression);
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

Value *Codegen::codegenPostfixExpr(const PostfixExprPtr &expr) {
  Value *obj = codegenExpr(expr->expression);
  if (std::holds_alternative<VarExprPtr>(expr->expression)) {
    const auto &varExpr = std::get<VarExprPtr>(expr->expression);
    postfixExpr(expr->op, obj, envManager.get(varExpr->ident));
  }
  return obj;
}

Value *Codegen::codegenVarStmt(const VarStmtPtr &stmt) {
  Value *value = codegenExpr(stmt->initializer);
  AllocaInst *varInst = getOrCreateAllocVar(stmt->varName);
  builder->CreateStore(value, varInst);
  return value;
};

Value *Codegen::codegenIfStmt(const IfStmtPtr &stmt) {
  Value *conditionV = codegenExpr(stmt->condition);
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
  codegenStmt(stmt->thenBranch);
  builder->CreateBr(mergeBB);
  thenBB = builder->GetInsertBlock();

  if (stmt->elseBranch) {
    function->insert(function->end(), elseBB);
    builder->SetInsertPoint(elseBB);
    codegenStmt(*stmt->elseBranch);
    builder->CreateBr(mergeBB);
    elseBB = builder->GetInsertBlock();
  }

  function->insert(function->end(), mergeBB);
  builder->SetInsertPoint(mergeBB);
  return nullptr;
}

Value *Codegen::codegenBlockStmt(const BlockStmtPtr &stmt) {
  auto curEnv = envManager.getCurEnv();
  envManager.createNewEnv();
  executeStmts(stmt->statements);
  envManager.discardEnvsTill(curEnv);
  return nullptr;
}

Value *Codegen::codegenWhileStmt(const WhileStmtPtr &stmt) {
  Function *function = builder->GetInsertBlock()->getParent();

  BasicBlock *conditionBB = BasicBlock::Create(*context, "condition", function);
  BasicBlock *loopBB = BasicBlock::Create(*context, "loop", function);
  BasicBlock *afterBB = BasicBlock::Create(*context, "afterloop", function);

  builder->CreateBr(conditionBB);
  builder->SetInsertPoint(conditionBB);
  Value *conditionV = codegenExpr(stmt->condition);
  conditionV = builder->CreateICmpNE(
      conditionV, ConstantInt::get(llvm::Type::getInt1Ty(*context), 0),
      "loopcondition");
  builder->CreateCondBr(conditionV, loopBB, afterBB);

  builder->SetInsertPoint(loopBB);
  codegenStmt(stmt->body);
  builder->CreateBr(conditionBB);

  builder->SetInsertPoint(afterBB);
  return nullptr;
}

Value *Codegen::codegenPrintStmt(const PrintStmtPtr &stmt) {
  Value *val = codegenExpr(stmt->value);
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

Value *Codegen::codegenExprStmt(const ExprStmtPtr &stmt) {
  codegenExpr(stmt->expression);
  return nullptr;
}

AllocaInst *Codegen::allocVar(std::string_view name) {
  BasicBlock *insertBB = builder->GetInsertBlock();
  Function *func = insertBB->getParent();
  builder->SetInsertPoint(&func->getEntryBlock(),
                          func->getEntryBlock().begin());
  AllocaInst *inst = builder->CreateAlloca(intType, 0, name);
  builder->SetInsertPoint(insertBB);
  return inst;
}

AllocaInst *Codegen::getOrCreateAllocVar(const Token &variable) {
  if (envManager.contains(variable))
    return envManager.get(variable);
  auto inst = allocVar(variable.getLexeme());
  envManager.define(variable, inst);
  return inst;
}

} // namespace prsl::Codegen