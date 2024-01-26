#pragma once

#include "prsl/AST/NodeTypes.hpp"

// Не все компиляторы еще поддерживают стандартный unreachable,
// Так что пусть будет такой
[[noreturn]] inline void unreachable()
{
    // Uses compiler specific extensions if possible.
    // Even if no extension is used, undefined behavior is still raised by
    // an empty function body and the noreturn attribute.
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
    __assume(false);
#else // GCC, Clang
    __builtin_unreachable();
#endif
}

namespace prsl::AST {

template <typename ExprVisitRes = void, typename StmtVisitRes = void>
class ASTVisitor {
public:
  ExprVisitRes visitExpr(const ExprPtrVariant &expr) {
    switch (expr.index()) {
    case 0:
      return visitLiteralExpr(std::get<0>(expr));
    case 1:
      return visitGroupingExpr(std::get<1>(expr));
    case 2:
      return visitVarExpr(std::get<2>(expr));
    case 3:
      return visitInputExpr(std::get<3>(expr));
    case 4:
      return visitAssignmentExpr(std::get<4>(expr));
    case 5:
      return visitUnaryExpr(std::get<5>(expr));
    case 6:
      return visitBinaryExpr(std::get<6>(expr));
    case 7:
      return visitPostfixExpr(std::get<7>(expr));
    default:
      unreachable();
    }
  }
  StmtVisitRes visitStmt(const StmtPtrVariant &stmt) {
    switch (stmt.index()) {
    case 0:
      return visitVarStmt(std::get<0>(stmt));
    case 1:
      return visitIfStmt(std::get<1>(stmt));
    case 2:
      return visitBlockStmt(std::get<2>(stmt));
    case 3:
      return visitWhileStmt(std::get<3>(stmt));
    case 4:
      return visitPrintStmt(std::get<4>(stmt));
    case 5:
      return visitExprStmt(std::get<5>(stmt));
    case 6:
      return visitFunctionStmt(std::get<6>(stmt));
    default:
      unreachable();
    }
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

  virtual StmtVisitRes visitVarStmt(const VarStmtPtr &stmt) = 0;
  virtual StmtVisitRes visitIfStmt(const IfStmtPtr &stmt) = 0;
  virtual StmtVisitRes visitBlockStmt(const BlockStmtPtr &stmt) = 0;
  virtual StmtVisitRes visitWhileStmt(const WhileStmtPtr &stmt) = 0;
  virtual StmtVisitRes visitPrintStmt(const PrintStmtPtr &stmt) = 0;
  virtual StmtVisitRes visitExprStmt(const ExprStmtPtr &stmt) = 0;
  virtual StmtVisitRes visitFunctionStmt(const FunctionStmtPtr &stmt) = 0;
};

} // namespace prsl::AST