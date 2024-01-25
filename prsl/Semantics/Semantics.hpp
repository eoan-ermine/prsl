#pragma once

#include "prsl/AST/NodeTypes.hpp"
#include "prsl/Debug/ErrorReporter.hpp"
#include "prsl/Evaluator/Environment.hpp"
#include "prsl/Types/Token.hpp"

namespace prsl::Semantics {

using namespace AST;

using Errors::ErrorReporter;

using Types::Token;
using Type = Types::Token::Type;

class Semantics {
public:
  explicit Semantics(ErrorReporter &eReporter);

  void resolveExpr(const ExprPtrVariant &expr);
  void resolveStmt(const StmtPtrVariant &stmt);

  void executeStmts(const std::vector<StmtPtrVariant> &stmts);
  void dump(std::string_view filename);

private:
  void resolveLiteralExpr(const LiteralExprPtr &expr);
  void resolveGroupingExpr(const GroupingExprPtr &expr);
  void resolveVarExpr(const VarExprPtr &expr);
  void resolveInputExpr(const InputExprPtr &expr);
  void resolveAssignmentExpr(const AssignmentExprPtr &expr);
  void resolveUnaryExpr(const UnaryExprPtr &expr);
  void resolveBinaryExpr(const BinaryExprPtr &expr);
  void resolvePostfixExpr(const PostfixExprPtr &expr);

  void resolveVarStmt(const VarStmtPtr &stmt);
  void resolveIfStmt(const IfStmtPtr &stmt);
  void resolveBlockStmt(const BlockStmtPtr &stmt);
  void resolveWhileStmt(const WhileStmtPtr &stmt);
  void resolvePrintStmt(const PrintStmtPtr &stmt);
  void resolveExprStmt(const ExprStmtPtr &stmt);

  ErrorReporter &eReporter;
  Evaluator::EnvironmentManager<bool> envManager;
};

} // namespace prsl::Semantics