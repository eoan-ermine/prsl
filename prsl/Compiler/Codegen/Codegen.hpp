#pragma once

#include "prsl/AST/ASTVisitor.hpp"
#include "prsl/AST/NodeTypes.hpp"
#include "prsl/Compiler/Common/Environment.hpp"
#include "prsl/Compiler/Common/FunctionsManager.hpp"
#include "prsl/Compiler/CompilerFlags.hpp"
#include "prsl/Debug/ErrorReporter.hpp"
#include "prsl/Parser/Token.hpp"

#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Target/TargetMachine.h>

#include <filesystem>
#include <stack>

namespace prsl::Codegen {

using namespace llvm;

using namespace AST;

using Errors::ErrorReporter;

using Types::Token;
using Type = Types::Token::Type;

class Codegen : public ASTVisitor<Value *, Value *> {
public:
  explicit Codegen(Compiler::CompilerFlags *flags, ErrorReporter &eReporter);
  bool dump(const std::filesystem::path &path) const;

private:
  Value *visitLiteralExpr(const LiteralExprPtr &expr) override;
  Value *visitGroupingExpr(const GroupingExprPtr &expr) override;
  Value *visitVarExpr(const VarExprPtr &expr) override;
  Value *visitInputExpr(const InputExprPtr &expr) override;
  Value *visitAssignmentExpr(const AssignmentExprPtr &expr) override;
  Value *visitUnaryExpr(const UnaryExprPtr &expr) override;
  Value *visitBinaryExpr(const BinaryExprPtr &expr) override;
  Value *visitPostfixExpr(const PostfixExprPtr &expr) override;
  Value *visitScopeExpr(const ScopeExprPtr &stmt) override;
  Value *visitFuncExpr(const FuncExprPtr &expr) override;
  Value *visitCallExpr(const CallExprPtr &expr) override;

  Value *visitVarStmt(const VarStmtPtr &stmt) override;
  Value *visitIfStmt(const IfStmtPtr &stmt) override;
  Value *visitWhileStmt(const WhileStmtPtr &stmt) override;
  Value *visitPrintStmt(const PrintStmtPtr &stmt) override;
  Value *visitExprStmt(const ExprStmtPtr &stmt) override;
  Value *visitFunctionStmt(const FunctionStmtPtr &stmt) override;
  Value *visitBlockStmt(const BlockStmtPtr &stmt) override;
  Value *visitReturnStmt(const ReturnStmtPtr &stmt) override;
  Value *visitNullStmt(const NullStmtPtr &stmt) override;

  Value *postfixExpr(const Token &op, Value *obj, Value *res);
  AllocaInst *allocVar(std::string_view name);
  AllocaInst *getOrCreateAllocVar(const Token &variable);
  AllocaInst *getAllocVar(const Token &ident);
  Function *getFunction(const Token &ident);
  Value *evaluateScope(const ScopeExprPtr &stmt);

  Compiler::CompilerFlags *flags{nullptr};
  Compiler::OutputFileType type;
  llvm::PassBuilder passBuilder;
  llvm::OptimizationLevel optLevel;
  llvm::TargetMachine *targetMachine{nullptr};

  ErrorReporter &eReporter;
  std::unique_ptr<LLVMContext> context;
  std::unique_ptr<IRBuilder<>> builder;
  std::unique_ptr<Module> module;
  Types::EnvironmentManager<Value *> envManager;
  Types::FunctionsManager<Function *> functionsManager;
  llvm::Type *intType;
  struct RetVal {
    Value *value;
    bool isFunction;
  };
  std::stack<RetVal> returnStack;
};

} // namespace prsl::Codegen