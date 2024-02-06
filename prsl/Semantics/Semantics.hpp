#pragma once

#include "prsl/AST/ASTVisitor.hpp"
#include "prsl/AST/NodeTypes.hpp"
#include "prsl/Debug/ErrorReporter.hpp"
#include "prsl/Evaluator/Environment.hpp"
#include "prsl/Types/Token.hpp"
#include <string_view>
#include <unordered_map>

namespace prsl::Semantics {

using namespace AST;

using Errors::ErrorReporter;

using Types::Token;
using Type = Types::Token::Type;

class Semantics : public ASTVisitor<> {
public:
  explicit Semantics(ErrorReporter &eReporter);
  bool dump(std::string_view filename) override;

private:
  void visitLiteralExpr(const LiteralExprPtr &expr) override;
  void visitGroupingExpr(const GroupingExprPtr &expr) override;
  void visitVarExpr(const VarExprPtr &expr) override;
  void visitInputExpr(const InputExprPtr &expr) override;
  void visitAssignmentExpr(const AssignmentExprPtr &expr) override;
  void visitUnaryExpr(const UnaryExprPtr &expr) override;
  void visitBinaryExpr(const BinaryExprPtr &expr) override;
  void visitPostfixExpr(const PostfixExprPtr &expr) override;
  void visitScopeExpr(const ScopeExprPtr &expr) override;
  void visitFuncExpr(const FuncExprPtr &expr) override;
  void visitCallExpr(const CallExprPtr &expr) override;

  void visitVarStmt(const VarStmtPtr &stmt) override;
  void visitIfStmt(const IfStmtPtr &stmt) override;
  void visitWhileStmt(const WhileStmtPtr &stmt) override;
  void visitPrintStmt(const PrintStmtPtr &stmt) override;
  void visitExprStmt(const ExprStmtPtr &stmt) override;
  void visitFunctionStmt(const FunctionStmtPtr &stmt) override;
  void visitBlockStmt(const BlockStmtPtr &stmt) override;

  ErrorReporter &eReporter;
  struct VarState {
    bool isInit;
    enum Type { VAR, FUNCTION } type;
  };
  Evaluator::EnvironmentManager<VarState> envManager;
  std::unordered_map<std::string_view, bool> functionsManager;
};

} // namespace prsl::Semantics