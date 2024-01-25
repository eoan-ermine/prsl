#pragma once

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
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/FileSystem.h>
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
  explicit Codegen(ErrorReporter &eReporter);

  Value *codegenExpr(const ExprPtrVariant &expr);
  Value *codegenStmt(const StmtPtrVariant &stmt);

  void executeStmts(const std::vector<StmtPtrVariant> &stmts);
  void dump(std::string_view filename);

private:
  Value *codegenLiteralExpr(const LiteralExprPtr &expr);
  Value *codegenGroupingExpr(const GroupingExprPtr &expr);
  Value *codegenVarExpr(const VarExprPtr &expr);
  Value *codegenInputExpr(const InputExprPtr &expr);
  Value *codegenAssignmentExpr(const AssignmentExprPtr &expr);
  Value *codegenUnaryExpr(const UnaryExprPtr &expr);
  Value *codegenBinaryExpr(const BinaryExprPtr &expr);
  Value *postfixExpr(const Token &op, Value *obj, Value *res);
  Value *codegenPostfixExpr(const PostfixExprPtr &expr);

  Value *codegenVarStmt(const VarStmtPtr &stmt);
  Value *codegenIfStmt(const IfStmtPtr &stmt);
  Value *codegenBlockStmt(const BlockStmtPtr &stmt);
  Value *codegenWhileStmt(const WhileStmtPtr &stmt);
  Value *codegenPrintStmt(const PrintStmtPtr &stmt);
  Value *codegenExprStmt(const ExprStmtPtr &stmt);

  AllocaInst *allocVar(std::string_view name);
  AllocaInst *getOrCreateAllocVar(const Token &variable);

  ErrorReporter &eReporter;
  std::unique_ptr<LLVMContext> context;
  std::unique_ptr<IRBuilder<>> builder;
  std::unique_ptr<Module> module;
  Evaluator::EnvironmentManager<AllocaInst *> envManager;
  llvm::Type *intType;
};

} // namespace prsl::Codegen