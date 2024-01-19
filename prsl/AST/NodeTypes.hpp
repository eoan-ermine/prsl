#pragma once

#include "../Types/Token.hpp"
#include <memory>
#include <optional>
#include <variant>
#include <vector>

namespace prsl::AST {

struct LiteralExpr;

struct GroupingExpr;

struct VarExpr;

struct InputExpr;

struct AssignmentExpr;

struct UnaryExpr;

struct BinaryExpr;

struct PostfixExpr;

using LiteralExprPtr = std::unique_ptr<LiteralExpr>;

using GroupingExprPtr = std::unique_ptr<GroupingExpr>;

using VarExprPtr = std::unique_ptr<VarExpr>;

using InputExprPtr = std::unique_ptr<InputExpr>;

using AssignmentExprPtr = std::unique_ptr<AssignmentExpr>;

using UnaryExprPtr = std::unique_ptr<UnaryExpr>;

using BinaryExprPtr = std::unique_ptr<BinaryExpr>;

using PostfixExprPtr = std::unique_ptr<PostfixExpr>;

using ExprPtrVariant =
    std::variant<LiteralExprPtr, GroupingExprPtr, VarExprPtr, InputExprPtr,
                 AssignmentExprPtr, UnaryExprPtr, BinaryExprPtr,
                 PostfixExprPtr>;

struct VarStmt;

struct IfStmt;

struct BlockStmt;

struct WhileStmt;

using VarStmtPtr = std::unique_ptr<VarStmt>;

using IfStmtPtr = std::unique_ptr<IfStmt>;

using BlockStmtPtr = std::unique_ptr<BlockStmt>;

using WhileStmtPtr = std::unique_ptr<WhileStmt>;

using StmtPtrVariant =
    std::variant<VarStmtPtr, IfStmtPtr, BlockStmtPtr, WhileStmtPtr>;

using prsl::Types::Token;

struct LiteralExpr final {
  int literalVal;
  explicit LiteralExpr(int value) { literalVal = value; }
};

struct GroupingExpr final {
  ExprPtrVariant expression;
  explicit GroupingExpr(ExprPtrVariant expression)
      : expression(std::move(expression)) {}
};

struct VarExpr final {
  Token ident;
  explicit VarExpr(Token ident) : ident(ident) {}
};

struct InputExpr final {
  explicit InputExpr() {}
};

struct AssignmentExpr final {
  Token varName;
  ExprPtrVariant initializer;
  explicit AssignmentExpr(Token varName, ExprPtrVariant initializer)
      : varName(varName), initializer(std::move(initializer)) {}
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

struct VarStmt final {
  Token varName;
  ExprPtrVariant initializer;
  explicit VarStmt(Token varName, ExprPtrVariant initializer)
      : varName(varName), initializer(std::move(initializer)) {}
};

struct IfStmt final {
  ExprPtrVariant condition;
  StmtPtrVariant thenBranch;
  std::optional<StmtPtrVariant> elseBranch;

  explicit IfStmt(ExprPtrVariant condition, StmtPtrVariant thenBranch,
                  std::optional<StmtPtrVariant> elseBranch)
      : condition(std::move(condition)), thenBranch(std::move(thenBranch)),
        elseBranch(std::move(elseBranch)) {}
};

struct BlockStmt final {
  std::vector<StmtPtrVariant> statements;
  explicit BlockStmt(std::vector<StmtPtrVariant> statements)
      : statements(std::move(statements)) {}
};

struct WhileStmt final {
  ExprPtrVariant condition;
  StmtPtrVariant body;
  explicit WhileStmt(ExprPtrVariant condition, StmtPtrVariant body)
      : condition(std::move(condition)), body(std::move(body)) {}
};

inline auto createLiteralEPV(int literalVal) -> ExprPtrVariant {
  return std::make_unique<LiteralExpr>(literalVal);
}

inline auto createGroupingEPV(ExprPtrVariant expression) -> ExprPtrVariant {
  return std::make_unique<GroupingExpr>(std::move(expression));
}

inline auto createVarEPV(Token ident) -> ExprPtrVariant {
  return std::make_unique<VarExpr>(ident);
}

inline auto createInputEPV() -> ExprPtrVariant {
  return std::make_unique<InputExpr>();
}

inline auto createAssignmentEPV(Token varName,
                                ExprPtrVariant initializer) -> ExprPtrVariant {
  return std::make_unique<AssignmentExpr>(varName, std::move(initializer));
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

inline auto createIfSPV(ExprPtrVariant condition, StmtPtrVariant thenBranch,
                        std::optional<StmtPtrVariant> elseBranch) {
  return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch),
                                  std::move(elseBranch));
}

inline auto createBlockSPV(std::vector<StmtPtrVariant> statements) {
  return std::make_unique<BlockStmt>(std::move(statements));
}

inline auto createWhileSPV(ExprPtrVariant condition, StmtPtrVariant body) {
  return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

} // namespace prsl::AST