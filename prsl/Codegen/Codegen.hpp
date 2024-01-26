#pragma once

#include "prsl/AST/ASTVisitor.hpp"
#include "prsl/AST/NodeTypes.hpp"
#include "prsl/Debug/ErrorReporter.hpp"
#include "prsl/Evaluator/Environment.hpp"
#include "prsl/Types/Token.hpp"

#include "llvm/IR/BasicBlock.h"
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

namespace prsl::Codegen {

using namespace llvm;

using namespace AST;

using Errors::ErrorReporter;

using Types::Token;
using Type = Types::Token::Type;

class Codegen : public ASTVisitor<Value *, Value *> {
public:
  explicit Codegen(ErrorReporter &eReporter);
  bool dump(std::string_view filename) override;

private:
  Value *visitLiteralExpr(const LiteralExprPtr &expr) override;
  Value *visitGroupingExpr(const GroupingExprPtr &expr) override;
  Value *visitVarExpr(const VarExprPtr &expr) override;
  Value *visitInputExpr(const InputExprPtr &expr) override;
  Value *visitAssignmentExpr(const AssignmentExprPtr &expr) override;
  Value *visitUnaryExpr(const UnaryExprPtr &expr) override;
  Value *visitBinaryExpr(const BinaryExprPtr &expr) override;
  Value *visitPostfixExpr(const PostfixExprPtr &expr) override;

  Value *visitVarStmt(const VarStmtPtr &stmt) override;
  Value *visitIfStmt(const IfStmtPtr &stmt) override;
  Value *visitBlockStmt(const BlockStmtPtr &stmt) override;
  Value *visitWhileStmt(const WhileStmtPtr &stmt) override;
  Value *visitPrintStmt(const PrintStmtPtr &stmt) override;
  Value *visitExprStmt(const ExprStmtPtr &stmt) override;
  Value *visitFunctionStmt(const FunctionStmtPtr &stmt) override;

  Value *postfixExpr(const Token &op, Value *obj, Value *res);
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