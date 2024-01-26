#include "Semantics.hpp"

#include "prsl/Debug/RuntimeError.hpp"
#include <utility>
#include <variant>

namespace prsl::Semantics {

Semantics::Semantics(ErrorReporter &eReporter)
    : eReporter(eReporter), envManager(eReporter) {}

void Semantics::resolveExpr(const ExprPtrVariant &expr) {
  switch (expr.index()) {
  case 0:
    return resolveLiteralExpr(std::get<0>(expr));
  case 1:
    return resolveGroupingExpr(std::get<1>(expr));
  case 2:
    return resolveVarExpr(std::get<2>(expr));
  case 3:
    return resolveInputExpr(std::get<3>(expr));
  case 4:
    return resolveAssignmentExpr(std::get<4>(expr));
  case 5:
    return resolveUnaryExpr(std::get<5>(expr));
  case 6:
    return resolveBinaryExpr(std::get<6>(expr));
  case 7:
    return resolvePostfixExpr(std::get<7>(expr));
  default:
    std::unreachable();
  }
}

void Semantics::resolveStmt(const StmtPtrVariant &stmt) {
  switch (stmt.index()) {
  case 0:
    return resolveVarStmt(std::get<0>(stmt));
  case 1:
    return resolveIfStmt(std::get<1>(stmt));
  case 2:
    return resolveBlockStmt(std::get<2>(stmt));
  case 3:
    return resolveWhileStmt(std::get<3>(stmt));
  case 4:
    return resolvePrintStmt(std::get<4>(stmt));
  case 5:
    return resolveExprStmt(std::get<5>(stmt));
  case 6:
    return resolveFunctionStmt(std::get<6>(stmt));
  default:
    std::unreachable();
  }
}

void Semantics::execute(const StmtPtrVariant &stmt) {
  try {
    resolveStmt(stmt);
  } catch (const Errors::RuntimeError &e) {
    return;
  }
}

void Semantics::dump(std::string_view filename) {}

void Semantics::resolveLiteralExpr(const LiteralExprPtr &expr) { return; }

void Semantics::resolveGroupingExpr(const GroupingExprPtr &expr) {
  resolveExpr(expr->expression);
}

void Semantics::resolveVarExpr(const VarExprPtr &expr) {
  if (!envManager.contains(expr->ident))
    throw reportRuntimeError(eReporter, expr->ident,
                             "Attempt to access an undef variable");
  if (envManager.get(expr->ident) == false) {
    throw reportRuntimeError(eReporter, expr->ident,
                             "Can't read variable in its own initializer");
  }
}

void Semantics::resolveInputExpr(const InputExprPtr &expr) { return; }

void Semantics::resolveAssignmentExpr(const AssignmentExprPtr &expr) {
  if (!envManager.contains(expr->varName))
    envManager.define(expr->varName, false);
  resolveExpr(expr->initializer);
  envManager.assign(expr->varName, true);
}

void Semantics::resolveUnaryExpr(const UnaryExprPtr &expr) {
  resolveExpr(expr->expression);
}

void Semantics::resolveBinaryExpr(const BinaryExprPtr &expr) {
  resolveExpr(expr->lhsExpression);
  resolveExpr(expr->rhsExpression);
}

void Semantics::resolvePostfixExpr(const PostfixExprPtr &expr) {
  const auto &expression = expr->expression;
  if (std::holds_alternative<VarExprPtr>(expression) ||
      std::holds_alternative<AssignmentExprPtr>(expression))
    resolveExpr(expression);
  else
    throw reportRuntimeError(eReporter, expr->op, "Illegal postfix expression");
}

void Semantics::resolveVarStmt(const VarStmtPtr &stmt) {
  if (!envManager.contains(stmt->varName)) {
    envManager.define(stmt->varName, false);
  }
  resolveExpr(stmt->initializer);
  envManager.assign(stmt->varName, true);
}

void Semantics::resolveIfStmt(const IfStmtPtr &stmt) {
  resolveExpr(stmt->condition);
  resolveStmt(stmt->thenBranch);
  if (stmt->elseBranch)
    resolveStmt(*stmt->elseBranch);
}

void Semantics::resolveBlockStmt(const BlockStmtPtr &stmt) {
  for (const auto &stmt : stmt->statements) {
    resolveStmt(stmt);
  }
}

void Semantics::resolveWhileStmt(const WhileStmtPtr &stmt) {
  resolveExpr(stmt->condition);
  resolveStmt(stmt->body);
}

void Semantics::resolvePrintStmt(const PrintStmtPtr &stmt) {
  resolveExpr(stmt->value);
}

void Semantics::resolveExprStmt(const ExprStmtPtr &stmt) {
  resolveExpr(stmt->expression);
}

void Semantics::resolveFunctionStmt(const FunctionStmtPtr &stmt) {
  for (const auto &stmt : stmt->body) {
    resolveStmt(stmt);
  }
}

} // namespace prsl::Semantics