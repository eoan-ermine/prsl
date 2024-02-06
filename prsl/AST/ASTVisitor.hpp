#pragma once

#include "prsl/AST/NodeTypes.hpp"
#include "prsl/Utils/Utils.hpp"

namespace prsl::AST {

template <typename ExprVisitRes = void, typename StmtVisitRes = void>
class ASTVisitor {
public:
  ExprVisitRes visitExpr(const ExprPtrVariant &expr) {
    return std::visit<ExprVisitRes>(Utils::select{
      [&](const LiteralExprPtr &expr) { return visitLiteralExpr(expr); },
      [&](const GroupingExprPtr &expr) { return visitGroupingExpr(expr); },
      [&](const VarExprPtr &expr) { return visitVarExpr(expr); },
      [&](const InputExprPtr &expr) { return visitInputExpr(expr); },
      [&](const AssignmentExprPtr &expr) { return visitAssignmentExpr(expr); },
      [&](const UnaryExprPtr &expr) { return visitUnaryExpr(expr); },
      [&](const BinaryExprPtr &expr) { return visitBinaryExpr(expr); },
      [&](const PostfixExprPtr &expr) { return visitPostfixExpr(expr); },
      [&](const ScopeExprPtr &expr) { return visitScopeExpr(expr); },
      [&](const FuncExprPtr &expr) { return visitFuncExpr(expr); },
      [&](const CallExprPtr &expr) { return visitCallExpr(expr); }
    }, expr);
  }
  StmtVisitRes visitStmt(const StmtPtrVariant &stmt) {
    return std::visit<StmtVisitRes>(Utils::select{
      [&](const VarStmtPtr &stmt) { return visitVarStmt(stmt); },
      [&](const IfStmtPtr &stmt) { return visitIfStmt(stmt); },
      [&](const WhileStmtPtr &stmt) { return visitWhileStmt(stmt); },
      [&](const PrintStmtPtr &stmt) { return visitPrintStmt(stmt); },
      [&](const ExprStmtPtr &stmt) { return visitExprStmt(stmt); },
      [&](const FunctionStmtPtr &stmt) { return visitFunctionStmt(stmt); },
      [&](const BlockStmtPtr &stmt) { return visitBlockStmt(stmt); },
      [&](const ReturnStmtPtr &stmt) { return visitReturnStmt(stmt); }
    }, stmt);
  }
  virtual bool dump(std::string_view) = 0;

protected:
  virtual ExprVisitRes visitLiteralExpr(const LiteralExprPtr &expr) = 0;
  virtual ExprVisitRes visitGroupingExpr(const GroupingExprPtr &expr) = 0;
  virtual ExprVisitRes visitVarExpr(const VarExprPtr &expr) = 0;
  virtual ExprVisitRes visitInputExpr(const InputExprPtr &expr) = 0;
  virtual ExprVisitRes visitAssignmentExpr(const AssignmentExprPtr &expr) = 0;
  virtual ExprVisitRes visitUnaryExpr(const UnaryExprPtr &expr) = 0;
  virtual ExprVisitRes visitBinaryExpr(const BinaryExprPtr &expr) = 0;
  virtual ExprVisitRes visitPostfixExpr(const PostfixExprPtr &expr) = 0;
  virtual ExprVisitRes visitScopeExpr(const ScopeExprPtr &expr) = 0;
  virtual ExprVisitRes visitFuncExpr(const FuncExprPtr &expr) = 0;
  virtual ExprVisitRes visitCallExpr(const CallExprPtr &expr) = 0;

  virtual StmtVisitRes visitVarStmt(const VarStmtPtr &stmt) = 0;
  virtual StmtVisitRes visitIfStmt(const IfStmtPtr &stmt) = 0;
  virtual StmtVisitRes visitWhileStmt(const WhileStmtPtr &stmt) = 0;
  virtual StmtVisitRes visitPrintStmt(const PrintStmtPtr &stmt) = 0;
  virtual StmtVisitRes visitExprStmt(const ExprStmtPtr &stmt) = 0;
  virtual StmtVisitRes visitFunctionStmt(const FunctionStmtPtr &stmt) = 0;
  virtual StmtVisitRes visitBlockStmt(const BlockStmtPtr &stmt) = 0;
  virtual StmtVisitRes visitReturnStmt(const ReturnStmtPtr &stmt) = 0;
};

} // namespace prsl::AST