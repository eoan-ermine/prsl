#include "Semantics.hpp"

#include "prsl/Debug/RuntimeError.hpp"
#include "prsl/Evaluator/Environment.hpp"
#include <variant>

namespace prsl::Semantics {

Semantics::Semantics(ErrorReporter &eReporter)
    : eReporter(eReporter), envManager(eReporter) {}

bool Semantics::dump(std::string_view filename) { return false; }

void Semantics::visitLiteralExpr(const LiteralExprPtr &expr) { return; }

void Semantics::visitGroupingExpr(const GroupingExprPtr &expr) {
  visitExpr(expr->expression);
}

void Semantics::visitVarExpr(const VarExprPtr &expr) {
  if (!envManager.contains(expr->ident))
    throw reportRuntimeError(eReporter, expr->ident,
                             "Attempt to access an undef variable");
  if (envManager.get(expr->ident).isInit == false) {
    throw reportRuntimeError(eReporter, expr->ident,
                             "Can't read variable in its own initializer");
  }
}

void Semantics::visitInputExpr(const InputExprPtr &expr) { return; }

void Semantics::visitAssignmentExpr(const AssignmentExprPtr &expr) {
  if (!envManager.contains(expr->varName))
    envManager.define(expr->varName, {false, VarState::Type::VAR});
  visitExpr(expr->initializer);
  envManager.assign(expr->varName, {true, VarState::Type::VAR});
}

void Semantics::visitUnaryExpr(const UnaryExprPtr &expr) {
  visitExpr(expr->expression);
}

void Semantics::visitBinaryExpr(const BinaryExprPtr &expr) {
  visitExpr(expr->lhsExpression);
  visitExpr(expr->rhsExpression);
}

void Semantics::visitPostfixExpr(const PostfixExprPtr &expr) {
  const auto &expression = expr->expression;
  if (std::holds_alternative<VarExprPtr>(expression) ||
      std::holds_alternative<AssignmentExprPtr>(expression))
    visitExpr(expression);
  else
    throw reportRuntimeError(eReporter, expr->op, "Illegal postfix expression");
}

void Semantics::visitScopeExpr(const ScopeExprPtr &stmt) {
  envManager.withNewEnviron([&]() {
    for (const auto &stmt : stmt->statements) {
      visitStmt(stmt);
    }
  });
}

void Semantics::visitFuncExpr(const FuncExprPtr &expr) {
  auto funcEnv = std::make_shared<decltype(envManager)::EnvType>(nullptr);
  if (expr->name)
    envManager.define(*expr->name, {true, VarState::Type::FUNCTION});
  envManager.withNewEnviron(funcEnv, [&]() {
    if (expr->name)
      envManager.define(*expr->name, {true, VarState::Type::FUNCTION});
    for (const auto &token : expr->parameters) {
      envManager.define(token, {true, VarState::Type::VAR});
    }
    visitScopeExpr(std::get<ScopeExprPtr>(expr->body));
  });
}

void Semantics::visitCallExpr(const CallExprPtr &expr) {
  if (!envManager.contains(expr->ident))
    throw reportRuntimeError(eReporter, expr->ident,
                             "Attempt to access an undef function");
  for (const auto &argument : expr->arguments) {
    visitExpr(argument);
  }
}

void Semantics::visitVarStmt(const VarStmtPtr &stmt) {
  if (!envManager.contains(stmt->varName)) {
    envManager.define(stmt->varName, {false, VarState::Type::VAR});
  }
  visitExpr(stmt->initializer);
  envManager.assign(stmt->varName, {true, VarState::Type::VAR});
}

void Semantics::visitIfStmt(const IfStmtPtr &stmt) {
  visitExpr(stmt->condition);
  visitStmt(stmt->thenBranch);
  if (stmt->elseBranch)
    visitStmt(*stmt->elseBranch);
}

void Semantics::visitWhileStmt(const WhileStmtPtr &stmt) {
  visitExpr(stmt->condition);
  visitStmt(stmt->body);
}

void Semantics::visitPrintStmt(const PrintStmtPtr &stmt) {
  visitExpr(stmt->value);
}

void Semantics::visitExprStmt(const ExprStmtPtr &stmt) {
  visitExpr(stmt->expression);
}

void Semantics::visitFunctionStmt(const FunctionStmtPtr &stmt) {
  for (const auto &stmt : stmt->body) {
    visitStmt(stmt);
  }
}

void Semantics::visitBlockStmt(const BlockStmtPtr &stmt) {
  envManager.withNewEnviron([&]() {
    for (const auto &stmt : stmt->statements) {
      visitStmt(stmt);
    }
  });
}

} // namespace prsl::Semantics