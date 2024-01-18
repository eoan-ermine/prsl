#pragma once

#include "../Types/Token.hpp"
#include <memory>
#include <variant>

namespace prsl::AST {

struct LiteralExpr;

struct GroupingExpr;

using LiteralExprPtr = std::unique_ptr<LiteralExpr>;

using GroupingExprPtr = std::unique_ptr<GroupingExpr>;

using ExprPtrVariant = std::variant<LiteralExprPtr, GroupingExprPtr>;

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

inline auto createVarSPV(Token varName,
                         ExprPtrVariant initializer) -> StmtPtrVariant {
  return std::make_unique<VarStmt>(varName, std::move(initializer));
}

} // namespace prsl::AST