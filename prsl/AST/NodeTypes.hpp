#pragma once

#include "../Types/Token.hpp"
#include <memory>
#include <variant>

namespace prsl::AST {

struct LiteralExpr;

struct GroupingExpr;

struct UnaryExpr;

struct BinaryExpr;

struct PostfixExpr;

using LiteralExprPtr = std::unique_ptr<LiteralExpr>;

using GroupingExprPtr = std::unique_ptr<GroupingExpr>;

using UnaryExprPtr = std::unique_ptr<UnaryExpr>;

using BinaryExprPtr = std::unique_ptr<BinaryExpr>;

using PostfixExprPtr = std::unique_ptr<PostfixExpr>;

using ExprPtrVariant =
    std::variant<LiteralExprPtr, GroupingExprPtr, UnaryExprPtr, BinaryExprPtr,
                 PostfixExprPtr>;

struct VarStmt;

using VarStmtPtr = std::unique_ptr<VarStmt>;

using StmtPtrVariant = std::variant<VarStmtPtr>;

struct LiteralExpr final {
  int literalVal;
  explicit LiteralExpr(int value) { literalVal = value; }
};

struct GroupingExpr final {
  ExprPtrVariant expression;
  explicit GroupingExpr(ExprPtrVariant expression)
      : expression(std::move(expression)) {}
};

struct UnaryExpr final {
  Types::Token op;
  ExprPtrVariant expression;
  explicit UnaryExpr(ExprPtrVariant expression, Types::Token op)
      : expression(std::move(expression)), op(op) {}
};

struct BinaryExpr final {
  Types::Token op;
  ExprPtrVariant lhsExpression;
  ExprPtrVariant rhsExpression;
  explicit BinaryExpr(ExprPtrVariant lhsExpression, Types::Token op,
                      ExprPtrVariant rhsExpression)
      : lhsExpression(std::move(lhsExpression)), op(op),
        rhsExpression(std::move(rhsExpression)) {}
};

struct PostfixExpr final {
  Types::Token op;
  ExprPtrVariant expression;
  explicit PostfixExpr(ExprPtrVariant expression, Types::Token op)
      : expression(std::move(expression)), op(op) {}
};

using prsl::Types::Token;

struct VarStmt final {
  Token varName;
  ExprPtrVariant initializer;
  explicit VarStmt(Token varName, ExprPtrVariant initializer)
      : varName(varName), initializer(std::move(initializer)) {}
};

inline auto createLiteralEPV(int literalVal) -> ExprPtrVariant {
  return std::make_unique<LiteralExpr>(literalVal);
}

inline auto createGroupingEPV(ExprPtrVariant expression) -> ExprPtrVariant {
  return std::make_unique<GroupingExpr>(std::move(expression));
}

inline auto createUnaryEPV(ExprPtrVariant expression,
                           Types::Token op) -> ExprPtrVariant {
  return std::make_unique<UnaryExpr>(std::move(expression), op);
}

inline auto createBinaryEPV(ExprPtrVariant lhsExpression, Types::Token op,
                            ExprPtrVariant rhsExpression) -> ExprPtrVariant {
  return std::make_unique<BinaryExpr>(std::move(lhsExpression), op,
                                      std::move(rhsExpression));
}

inline auto createPostfixEPV(ExprPtrVariant expression,
                              Types::Token op) -> ExprPtrVariant {
  return std::make_unique<PostfixExpr>(std::move(expression), op);
}

inline auto createVarSPV(Token varName,
                         ExprPtrVariant initializer) -> StmtPtrVariant {
  return std::make_unique<VarStmt>(varName, std::move(initializer));
}

} // namespace prsl::AST