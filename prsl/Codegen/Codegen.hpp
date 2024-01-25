#include "prsl/AST/NodeTypes.hpp"
#include "prsl/Debug/ErrorReporter.hpp"
#include "prsl/Debug/RuntimeError.hpp"
#include "prsl/Evaluator/Environment.hpp"
#include "prsl/Types/Token.hpp"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/Instructions.h>
#include <memory>
#include <variant>

namespace prsl::Codegen {

using namespace llvm;

using namespace AST;

using Errors::ErrorReporter;

using Types::Token;
using Type = Types::Token::Type;

class Codegen {
public:
  explicit Codegen(ErrorReporter &eReporter)
      : eReporter(eReporter), context(std::make_unique<LLVMContext>()),
        module(std::make_unique<Module>("Prsl", *context)),
        builder(std::make_unique<IRBuilder<>>(*context)),
        envManager(this->eReporter), intType(llvm::Type::getInt32Ty(*context)) {
    FunctionType *FT = FunctionType::get(llvm::Type::getVoidTy(*context),
                                         std::vector<llvm::Type *>{}, false);
    Function *F =
        Function::Create(FT, Function::ExternalLinkage, "main", module.get());
    BasicBlock *BB = BasicBlock::Create(*context, "", F);
    builder->SetInsertPoint(BB);
  }

  auto codegenExpr(const ExprPtrVariant &expr) -> Value * {
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

  Value *codegenStmt(const StmtPtrVariant &stmt) {
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
    default:
      std::unreachable();
    }
  }

  auto executeStmts(const std::vector<StmtPtrVariant> &stmts) {
    for (const auto &stmt : stmts) {
      try {
        codegenStmt(stmt);
      } catch (const Errors::RuntimeError &e) {
        eReporter.printToErr();
      }
    }
  }

  auto dump(std::string_view filename) {
    builder->CreateRetVoid();

    std::error_code ec;
    auto fileStream = llvm::raw_fd_ostream(filename, ec,
                                           llvm::sys::fs::OpenFlags::OF_None);
    if (ec)
      std::cout << ec << '\n';

    module->print(fileStream, nullptr);
  }

private:
  auto codegenLiteralExpr(const LiteralExprPtr &expr) -> Value * {
    return ConstantInt::get(intType, expr->literalVal);
  }

  auto codegenGroupingExpr(const GroupingExprPtr &expr) -> Value * {
    return codegenExpr(expr->expression);
  }

  auto codegenVarExpr(const VarExprPtr &expr) -> Value * {
    AllocaInst *V = envManager.get(expr->ident);
    return builder->CreateLoad(intType, V, expr->ident.getLexeme());
  }

  auto codegenInputExpr(const InputExprPtr &expr) -> Value * {
    BasicBlock *insertBB = builder->GetInsertBlock();
    Function *func_scanf = module->getFunction("scanf");

    if (!func_scanf) {
      std::vector<llvm::Type *> ints(0, intType);
      FunctionType *funcType = FunctionType::get(intType, ints, false);
      func_scanf = Function::Create(funcType, Function::ExternalLinkage,
                                    "scanf", module.get());
      func_scanf->setCallingConv(CallingConv::C);
    }

    Value *str = builder->CreateGlobalStringPtr("%d");
    auto* temp_var = allocVar("inputtemp");
    std::vector<Value *> call_params;
    call_params.push_back(str);
    call_params.push_back(temp_var);

    CallInst *call =
        llvm::CallInst::Create(func_scanf, call_params, "calltmp", insertBB);
    return builder->CreateLoad(intType, temp_var, "inputres");
  }

  auto codegenAssignmentExpr(const AssignmentExprPtr &expr) -> Value * {
    Value *value = codegenExpr(expr->initializer);
    AllocaInst *varInst = getOrCreateAllocVar(expr->varName);
    builder->CreateStore(value, varInst);
    return value;
  }

  auto codegenUnaryExpr(const UnaryExprPtr &expr) -> Value * {
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

  auto codegenBinaryExpr(const BinaryExprPtr &expr) -> Value * {
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

  auto postfixExpr(const Token &op, Value *obj, Value *res) {
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

    builder->CreateStore(value, res);
  }

  auto codegenPostfixExpr(const PostfixExprPtr &expr) -> Value * {
    Value *obj = codegenExpr(expr->expression);
    if (std::holds_alternative<VarExprPtr>(expr->expression)) {
      const auto &varExpr = std::get<VarExprPtr>(expr->expression);
      postfixExpr(expr->op, obj, envManager.get(varExpr->ident));
    }
    return obj;
  }

  Value *codegenVarStmt(const VarStmtPtr &stmt) {
    Value *value = codegenExpr(stmt->initializer);
    AllocaInst *varInst = getOrCreateAllocVar(stmt->varName);
    builder->CreateStore(value, varInst);
    return value;
  };

  Value *codegenIfStmt(const IfStmtPtr &stmt) {
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

  Value *codegenBlockStmt(const BlockStmtPtr &stmt) {
    auto curEnv = envManager.getCurEnv();
    envManager.createNewEnviron();
    executeStmts(stmt->statements);
    envManager.discardEnvironsTill(curEnv);
    return nullptr;
  }

  Value *codegenWhileStmt(const WhileStmtPtr &stmt) {
    Function *function = builder->GetInsertBlock()->getParent();
    BasicBlock *preheaderBB = builder->GetInsertBlock();
    BasicBlock *loopBB = BasicBlock::Create(*context, "loop", function);

    builder->CreateBr(loopBB);
    builder->SetInsertPoint(loopBB);

    codegenStmt(stmt->body);

    Value *conditionV = codegenExpr(stmt->condition);
    conditionV = builder->CreateICmpNE(
        conditionV, ConstantInt::get(llvm::Type::getInt1Ty(*context), 0),
        "loopcondition");

    BasicBlock *afterBB = BasicBlock::Create(*context, "afterloop", function);
    builder->CreateCondBr(conditionV, loopBB, afterBB);
    builder->SetInsertPoint(afterBB);

    return nullptr;
  }

  Value *codegenPrintStmt(const PrintStmtPtr &stmt) {
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

  AllocaInst *allocVar(std::string_view name) {
    BasicBlock *insertBB = builder->GetInsertBlock();
    Function *func = insertBB->getParent();
    builder->SetInsertPoint(&func->getEntryBlock(),
                            func->getEntryBlock().begin());
    AllocaInst *inst = builder->CreateAlloca(intType, 0, name);
    builder->SetInsertPoint(insertBB);
    return inst;
  }

  AllocaInst *getOrCreateAllocVar(const Token &variable) {
    if (envManager.contains(variable))
      return envManager.get(variable);
    auto inst = allocVar(variable.getLexeme());
    envManager.define(variable, inst);
    return inst;
  }

  ErrorReporter &eReporter;
  std::unique_ptr<LLVMContext> context;
  std::unique_ptr<IRBuilder<>> builder;
  std::unique_ptr<Module> module;
  Evaluator::EnvironmentManager<AllocaInst *> envManager;
  llvm::Type *intType;
};

} // namespace prsl::Codegen