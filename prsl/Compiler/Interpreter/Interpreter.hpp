#pragma once

#include "prsl/AST/ASTVisitor.hpp"
#include "prsl/AST/NodeTypes.hpp"
#include "prsl/Compiler/Common/Environment.hpp"
#include "prsl/Compiler/Common/FunctionsManager.hpp"
#include "prsl/Compiler/CompilerFlags.hpp"
#include "prsl/Compiler/Interpreter/Objects.hpp"
#include "prsl/Debug/Logger.hpp"
#include "prsl/Parser/Token.hpp"

#include <filesystem>
#include <stack>

namespace prsl::Interpreter {

using namespace AST;

using Errors::Logger;

using Types::Token;
using Type = Types::Token::Type;

class Interpreter : public ASTVisitor<PrslObject> {
public:
  explicit Interpreter(Compiler::CompilerFlags *flags, Logger &logger);
  bool dump(const std::filesystem::path &path) const;

private:
  PrslObject visitLiteralExpr(const LiteralExprPtr &expr) override;
  PrslObject visitGroupingExpr(const GroupingExprPtr &expr) override;
  PrslObject visitVarExpr(const VarExprPtr &expr) override;
  PrslObject visitInputExpr(const InputExprPtr &expr) override;
  PrslObject visitAssignmentExpr(const AssignmentExprPtr &expr) override;
  PrslObject visitUnaryExpr(const UnaryExprPtr &expr) override;
  PrslObject visitBinaryExpr(const BinaryExprPtr &expr) override;
  PrslObject visitPostfixExpr(const PostfixExprPtr &expr) override;
  PrslObject visitScopeExpr(const ScopeExprPtr &expr) override;
  PrslObject visitFuncExpr(const FuncExprPtr &expr) override;
  PrslObject visitCallExpr(const CallExprPtr &expr) override;

  void visitVarStmt(const VarStmtPtr &stmt) override;
  void visitIfStmt(const IfStmtPtr &stmt) override;
  void visitWhileStmt(const WhileStmtPtr &stmt) override;
  void visitPrintStmt(const PrintStmtPtr &stmt) override;
  void visitExprStmt(const ExprStmtPtr &stmt) override;
  void visitFunctionStmt(const FunctionStmtPtr &stmt) override;
  void visitBlockStmt(const BlockStmtPtr &stmt) override;
  void visitReturnStmt(const ReturnStmtPtr &stmt) override;
  void visitNullStmt(const NullStmtPtr &stmt) override;

  int getInt(const Token &token, const PrslObject &obj) const;
  PrslObject evaluateScope(const ScopeExprPtr &scope);

private:
  Compiler::CompilerFlags *flags;
  Logger &logger;
  Types::EnvironmentManager<PrslObject> envManager;
  Types::FunctionsManager<PrslObject> functionsManager;
  std::stack<PrslObject> returnStack;
};

} // namespace prsl::Interpreter