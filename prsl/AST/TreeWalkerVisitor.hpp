#pragma once

#include "prsl/AST/ASTVisitor.hpp"

namespace prsl {

namespace AST {

class TreeWalkerVisitor : public prsl::AST::ASTVisitor<> {
public:
  virtual void visitLiteralExpr(const LiteralExprPtr &expr) override {}

  virtual void visitGroupingExpr(const GroupingExprPtr &expr) override {
    visitExpr(expr->expression);
  }

  virtual void visitVarExpr(const VarExprPtr &expr) override {}

  virtual void visitInputExpr(const InputExprPtr &expr) override {}

  virtual void visitAssignmentExpr(const AssignmentExprPtr &expr) override {
    visitExpr(expr->initializer);
  }

  virtual void visitUnaryExpr(const UnaryExprPtr &expr) override {
    visitExpr(expr->expression);
  }

  virtual void visitBinaryExpr(const BinaryExprPtr &expr) override {
    visitExpr(expr->lhsExpression);
    visitExpr(expr->rhsExpression);
  }

  virtual void visitPostfixExpr(const PostfixExprPtr &expr) override {
    visitExpr(expr->expression);
  }

  virtual void visitScopeExpr(const ScopeExprPtr &expr) override {
    for (const auto &stmt : expr->statements) {
      visitStmt(stmt);
    }
  }

  virtual void visitFuncExpr(const FuncExprPtr &expr) override {
    visitScopeExpr(std::get<ScopeExprPtr>(expr->body));
  }

  virtual void visitCallExpr(const CallExprPtr &expr) override {
    for (const auto &arg : expr->arguments) {
      visitExpr(arg);
    }
  }

  virtual void visitVarStmt(const VarStmtPtr &stmt) override {
    visitExpr(stmt->initializer);
  }

  virtual void visitIfStmt(const IfStmtPtr &stmt) override {
    visitExpr(stmt->condition);
    visitStmt(stmt->thenBranch);
    if (stmt->elseBranch)
      visitStmt(*stmt->elseBranch);
  }

  virtual void visitWhileStmt(const WhileStmtPtr &stmt) override {
    visitExpr(stmt->condition);
    visitStmt(stmt->body);
  }

  virtual void visitPrintStmt(const PrintStmtPtr &stmt) override {
    visitExpr(stmt->value);
  }

  virtual void visitExprStmt(const ExprStmtPtr &stmt) override {
    visitExpr(stmt->expression);
  }

  virtual void visitFunctionStmt(const FunctionStmtPtr &stmt) override {
    for (const auto &stmt : stmt->body) {
      visitStmt(stmt);
    }
  }

  virtual void visitBlockStmt(const BlockStmtPtr &stmt) override {
    for (const auto &stmt : stmt->statements) {
      visitStmt(stmt);
    }
  }

  virtual void visitReturnStmt(const ReturnStmtPtr &stmt) override {}
};

} // namespace AST

} // namespace prsl