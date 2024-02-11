#include "prsl/Semantics/Semantics.hpp"
#include "prsl/Debug/RuntimeError.hpp"

namespace prsl::Semantics {

Semantics::Semantics(Errors::ErrorReporter &eReporter)
    : eReporter(eReporter), envManager(eReporter) {}

bool Semantics::dump(std::string_view filename) const { return false; }

void Semantics::visitVarExpr(const VarExprPtr &expr) {
  if (!envManager.contains(expr->ident))
    throw reportRuntimeError(eReporter, expr->ident,
                             "Attempt to access an undef variable");
  if (envManager.get(expr->ident) == false) {
    throw reportRuntimeError(eReporter, expr->ident,
                             "Can't read variable in its own initializer");
  }
}

void Semantics::visitAssignmentExpr(const AssignmentExprPtr &expr) {
  if (!envManager.contains(expr->varName))
    envManager.define(expr->varName, false);
  TreeWalkerVisitor::visitAssignmentExpr(expr);
  envManager.assign(expr->varName, true);
}

void Semantics::visitPostfixExpr(const PostfixExprPtr &expr) {
  const auto &expression = expr->expression;
  if (!(std::holds_alternative<VarExprPtr>(expression) ||
        std::holds_alternative<AssignmentExprPtr>(expression)))
    throw reportRuntimeError(eReporter, expr->op, "Illegal postfix expression");
  TreeWalkerVisitor::visitPostfixExpr(expr);
}

void Semantics::visitScopeExpr(const ScopeExprPtr &stmt) {
  envManager.withNewEnviron([&]() { TreeWalkerVisitor::visitScopeExpr(stmt); });
}

void Semantics::visitFuncExpr(const FuncExprPtr &expr) {
  auto funcEnv = std::make_shared<decltype(envManager)::EnvType>(nullptr);
  if (expr->name) {
    functionsManager.set(expr->name->getLexeme(), true);
  }
  envManager.withNewEnviron(funcEnv, [&]() {
    for (const auto &token : expr->parameters) {
      envManager.define(token, true);
    }
    bool previousInFunction = inFunction;
    inFunction = true;
    TreeWalkerVisitor::visitFuncExpr(expr);
    inFunction = previousInFunction;
  });
}

void Semantics::visitCallExpr(const CallExprPtr &expr) {
  if (!functionsManager.contains(expr->ident.getLexeme()) &&
      !envManager.contains(expr->ident))
    throw reportRuntimeError(eReporter, expr->ident,
                             "Attempt to access an undef function");
  TreeWalkerVisitor::visitCallExpr(expr);
}

void Semantics::visitVarStmt(const VarStmtPtr &stmt) {
  if (!envManager.contains(stmt->varName)) {
    envManager.define(stmt->varName, false);
  }
  TreeWalkerVisitor::visitVarStmt(stmt);
  envManager.assign(stmt->varName, true);
}

void Semantics::visitBlockStmt(const BlockStmtPtr &stmt) {
  envManager.withNewEnviron([&]() { TreeWalkerVisitor::visitBlockStmt(stmt); });
}

void Semantics::visitReturnStmt(const ReturnStmtPtr &stmt) {
  if (!inFunction && stmt->isFunction) {
    throw reportRuntimeError(eReporter, stmt->retToken,
                             "Can't return from top-level code");
  }
  TreeWalkerVisitor::visitReturnStmt(stmt);
  visitExpr(stmt->retValue);
}

} // namespace prsl::Semantics