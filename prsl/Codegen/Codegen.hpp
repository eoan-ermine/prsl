#include "prsl/AST/NodeTypes.hpp"
#include "prsl/Debug/ErrorReporter.hpp"
#include "prsl/Debug/RuntimeError.hpp"
#include "prsl/Evaluator/Environment.hpp"
#include "prsl/Types/Token.hpp"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include <llvm-18/llvm/IR/CallingConv.h>
#include <llvm-18/llvm/IR/Instructions.h>
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
      : eReporter(eReporter), context(std::make_unique<LLVMContext>()), module(std::make_unique<Module>("Prsl", *context)), builder(std::make_unique<IRBuilder<>>(*context)),
        envManager(this->eReporter) {}

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

  auto codegenStmts(const std::vector<StmtPtrVariant> &stmts) {
    for (const auto &stmt : stmts) {
      try {
        codegenStmt(stmt);
      } catch (const Errors::RuntimeError &e) {
        eReporter.printToErr();
      }
    }
  }

  auto print() {
    module->print(errs(), nullptr);
  }

private:
  auto codegenLiteralExpr(const LiteralExprPtr &expr) -> Value * {
    return ConstantInt::get(*context, APInt(32, expr->literalVal, true));
  }

  auto codegenGroupingExpr(const GroupingExprPtr &expr) -> Value * {
    return codegenExpr(expr->expression);
  }

  auto codegenVarExpr(const VarExprPtr &expr) -> Value * {
    Value *V = envManager.get(expr->ident);
    return V;
  }

  auto codegenInputExpr(const InputExprPtr &expr) -> Value * {
    BasicBlock *insertBB = builder->GetInsertBlock();
    Function *func_scanf = module->getFunction("scanf");
    
    if (!func_scanf) {
      std::vector<llvm::Type*> ints(0, llvm::Type::getInt32Ty(*context));
      FunctionType *funcType = FunctionType::get(llvm::Type::getInt32Ty(*context), ints, false);
      func_scanf = Function::Create(funcType, Function::ExternalLinkage, "scanf", module.get());
      func_scanf->setCallingConv(CallingConv::C);
    }

    Value *str = builder->CreateGlobalStringPtr("%d");
    std::vector<Value*> call_params;
    call_params.push_back(str);

    CallInst *call = llvm::CallInst::Create(func_scanf, call_params, "calltmp", insertBB);
    return call;
  }

  auto codegenAssignmentExpr(const AssignmentExprPtr &expr) -> Value * {
    envManager.assign(expr->varName, codegenExpr(expr->initializer));
    return envManager.get(expr->varName);
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
      return builder->CreateICmpNE(lhs, rhs);
    case Token::Type::EQUAL_EQUAL:
      return builder->CreateICmpEQ(lhs, rhs);
    case Token::Type::LESS:
      return builder->CreateICmpSLT(lhs, rhs);
    case Token::Type::LESS_EQUAL:
      return builder->CreateICmpSLE(lhs, rhs);
    case Token::Type::GREATER:
      return builder->CreateICmpSGT(lhs, rhs);
    case Token::Type::GREATER_EQUAL:
      return builder->CreateICmpSGE(lhs, rhs);
    default:
      break;
    }

    throw reportRuntimeError(eReporter, expr->op,
                             "Illegal operator in expression");
  }

  auto postfixExpr(const Token &op, Value *obj) -> Value * {
    Value *constOne = ConstantInt::get(*context, APInt(32, 1, true));

    switch (op.getType()) {
    case Token::Type::PLUS_PLUS:
      return builder->CreateAdd(obj, constOne);
    case Token::Type::MINUS_MINUS:
      return builder->CreateSub(obj, constOne);
    default:
      break;
    }

    throw reportRuntimeError(eReporter, op, "Illegal operator in expression");
  }

  auto codegenPostfixExpr(const PostfixExprPtr &expr) -> Value * {
    Value *obj = codegenExpr(expr->expression);
    if (std::holds_alternative<VarExprPtr>(expr->expression)) {
      envManager.assign(std::get<VarExprPtr>(expr->expression)->ident,
                        postfixExpr(expr->op, obj));
    }
    return obj;
  }

  Value *codegenVarStmt(const VarStmtPtr &stmt) {
    auto V = codegenExpr(stmt->initializer);
    if (!V)
      return nullptr;
    envManager.define(stmt->varName, codegenExpr(stmt->initializer));
    return nullptr;
  };

  Value *codegenIfStmt(const IfStmtPtr &stmt) {
    Value *conditionV = codegenExpr(stmt->condition);
    if (!conditionV)
      return nullptr;

    conditionV = builder->CreateICmpNE(
        conditionV, ConstantInt::get(*context, APInt(32, 0, true)));

    Function *function = builder->GetInsertBlock()->getParent();
    BasicBlock *thenBB = BasicBlock::Create(*context, "then", function);
    BasicBlock *elseBB = BasicBlock::Create(*context, "else");
    BasicBlock *mergeBB = BasicBlock::Create(*context, "ifcont");

    builder->SetInsertPoint(thenBB);

    Value *thenV = codegenStmt(stmt->thenBranch);
    if (!thenV)
      return nullptr;

    builder->CreateBr(mergeBB);
    thenBB = builder->GetInsertBlock();

    function->insert(function->end(), elseBB);
    builder->SetInsertPoint(elseBB);

    Value *elseV = codegenStmt(*stmt->elseBranch);
    if (!elseV)
      return nullptr;

    builder->CreateBr(mergeBB);
    elseBB = builder->GetInsertBlock();

    function->insert(function->end(), mergeBB);
    builder->SetInsertPoint(mergeBB);
    PHINode *PN =
        builder->CreatePHI(llvm::Type::getInt32Ty(*context), 2, "iftmp");
    PN->addIncoming(thenV, thenBB);
    PN->addIncoming(elseV, elseBB);
    return PN;
  }

  Value *codegenBlockStmt(const BlockStmtPtr &stmt) {
    auto curEnv = envManager.getCurEnv();
    envManager.createNewEnviron();
    codegenStmts(stmt->statements);
    envManager.discardEnvironsTill(curEnv);
    return nullptr;
  }

  Value *codegenWhileStmt(const WhileStmtPtr &stmt) {
    Function *function = builder->GetInsertBlock()->getParent();
    BasicBlock *preheaderBB = builder->GetInsertBlock();
    BasicBlock *loopBB = BasicBlock::Create(*context, "loop", function);

    builder->CreateBr(loopBB);
    builder->SetInsertPoint(loopBB);

    if (!codegenStmt(stmt->body))
        return nullptr;

    Value *endCondition = codegenExpr(stmt->condition);
    if (!endCondition)
        return nullptr;

    endCondition = builder->CreateICmpNE(endCondition, ConstantInt::get(*context, APInt(32, 0, true)), "loopcondition");

    BasicBlock *afterLoopBB = builder->GetInsertBlock();
    BasicBlock *afterBB = BasicBlock::Create(*context, "afterloop", function);

    builder->CreateCondBr(endCondition, loopBB, afterBB);

    return nullptr;
  }

  Value *codegenPrintStmt(const PrintStmtPtr &stmt) {
    return nullptr;
  }

  ErrorReporter &eReporter;
  std::unique_ptr<LLVMContext> context;
  std::unique_ptr<IRBuilder<>> builder;
  std::unique_ptr<Module> module;
  Evaluator::EnvironmentManager<Value *> envManager;
};

} // namespace prsl::Codegen