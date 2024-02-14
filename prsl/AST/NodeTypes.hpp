#pragma once

#include "prsl/Parser/Token.hpp"

#include <memory>
#include <optional>
#include <variant>
#include <vector>

namespace prsl::AST {

struct LiteralExpr;
using LiteralExprPtr = std::unique_ptr<LiteralExpr>;

struct GroupingExpr;
using GroupingExprPtr = std::unique_ptr<GroupingExpr>;

struct VarExpr;
using VarExprPtr = std::unique_ptr<VarExpr>;

struct InputExpr;
using InputExprPtr = std::unique_ptr<InputExpr>;

struct AssignmentExpr;
using AssignmentExprPtr = std::unique_ptr<AssignmentExpr>;

struct UnaryExpr;
using UnaryExprPtr = std::unique_ptr<UnaryExpr>;

struct BinaryExpr;
using BinaryExprPtr = std::unique_ptr<BinaryExpr>;

struct PostfixExpr;
using PostfixExprPtr = std::unique_ptr<PostfixExpr>;

struct ScopeExpr;
using ScopeExprPtr = std::unique_ptr<ScopeExpr>;

struct FuncExpr;
using FuncExprPtr = std::unique_ptr<FuncExpr>;

struct CallExpr;
using CallExprPtr = std::unique_ptr<CallExpr>;

using ExprPtrVariant =
    std::variant<LiteralExprPtr, GroupingExprPtr, VarExprPtr, InputExprPtr,
                 AssignmentExprPtr, UnaryExprPtr, BinaryExprPtr, PostfixExprPtr,
                 ScopeExprPtr, FuncExprPtr, CallExprPtr>;

struct VarStmt;
using VarStmtPtr = std::unique_ptr<VarStmt>;

struct IfStmt;
using IfStmtPtr = std::unique_ptr<IfStmt>;

struct WhileStmt;
using WhileStmtPtr = std::unique_ptr<WhileStmt>;

struct PrintStmt;
using PrintStmtPtr = std::unique_ptr<PrintStmt>;

struct ExprStmt;
using ExprStmtPtr = std::unique_ptr<ExprStmt>;

struct FunctionStmt;
using FunctionStmtPtr = std::unique_ptr<FunctionStmt>;

struct BlockStmt;
using BlockStmtPtr = std::unique_ptr<BlockStmt>;

struct ReturnStmt;
using ReturnStmtPtr = std::unique_ptr<ReturnStmt>;

struct NullStmt;
using NullStmtPtr = std::unique_ptr<NullStmt>;

using StmtPtrVariant =
    std::variant<VarStmtPtr, IfStmtPtr, WhileStmtPtr, PrintStmtPtr, ExprStmtPtr,
                 FunctionStmtPtr, BlockStmtPtr, ReturnStmtPtr, NullStmtPtr>;

using prsl::Types::Token;

struct LiteralExpr final {
  int literalVal;
  explicit constexpr LiteralExpr(int value) noexcept;
};
ExprPtrVariant createLiteralEPV(int literalVal);

struct GroupingExpr final {
  ExprPtrVariant expression;
  explicit constexpr GroupingExpr(ExprPtrVariant expression) noexcept;
};
ExprPtrVariant createGroupingEPV(ExprPtrVariant expression);

struct VarExpr final {
  Token ident;
  explicit constexpr VarExpr(Token ident) noexcept;
};
ExprPtrVariant createVarEPV(Token ident);

struct InputExpr final {
  explicit constexpr InputExpr() noexcept;
};
ExprPtrVariant createInputEPV();

struct AssignmentExpr final {
  Token varName;
  ExprPtrVariant initializer;
  explicit constexpr AssignmentExpr(Token varName,
                                    ExprPtrVariant initializer) noexcept;
};
ExprPtrVariant createAssignmentEPV(Token varName, ExprPtrVariant initializer);

struct UnaryExpr final {
  Types::Token op;
  ExprPtrVariant expression;
  explicit constexpr UnaryExpr(ExprPtrVariant expression,
                               Types::Token op) noexcept;
};
ExprPtrVariant createUnaryEPV(ExprPtrVariant expression, Types::Token op);

struct BinaryExpr final {
  Types::Token op;
  ExprPtrVariant lhsExpression;
  ExprPtrVariant rhsExpression;
  explicit constexpr BinaryExpr(ExprPtrVariant lhsExpression, Types::Token op,
                                ExprPtrVariant rhsExpression) noexcept;
};
ExprPtrVariant createBinaryEPV(ExprPtrVariant lhsExpression, Types::Token op,
                               ExprPtrVariant rhsExpression);

struct PostfixExpr final {
  Types::Token op;
  ExprPtrVariant expression;
  explicit constexpr PostfixExpr(ExprPtrVariant expression,
                                 Types::Token op) noexcept;
};
ExprPtrVariant createPostfixEPV(ExprPtrVariant expression, Types::Token op);

struct ScopeExpr final {
  std::vector<StmtPtrVariant> statements;
  explicit constexpr ScopeExpr(std::vector<StmtPtrVariant> statements) noexcept;
};
ExprPtrVariant createScopeEPV(std::vector<StmtPtrVariant> statements);

struct FuncExpr final {
  // For diagnostics
  Token token;
  std::optional<Token> name;
  std::vector<Token> parameters;
  ExprPtrVariant body;
  std::optional<ExprPtrVariant> retExpr;
  constexpr FuncExpr(Token token, std::optional<Token> name,
                     std::vector<Token> parameters) noexcept;
};
ExprPtrVariant createFuncEPV(Token token, std::optional<Token> name,
                             std::vector<Token> parameters);

struct CallExpr final {
  Token ident;
  std::vector<ExprPtrVariant> arguments;
  explicit constexpr CallExpr(Token ident,
                              std::vector<ExprPtrVariant> arguments) noexcept;
};
ExprPtrVariant createCallEPV(Token ident,
                             std::vector<ExprPtrVariant> arguments);

struct VarStmt final {
  Token varName;
  ExprPtrVariant initializer;
  explicit constexpr VarStmt(Token varName,
                             ExprPtrVariant initializer) noexcept;
};
StmtPtrVariant createVarSPV(Token varName, ExprPtrVariant initializer);

struct IfStmt final {
  ExprPtrVariant condition;
  StmtPtrVariant thenBranch;
  std::optional<StmtPtrVariant> elseBranch;

  explicit constexpr IfStmt(ExprPtrVariant condition, StmtPtrVariant thenBranch,
                            std::optional<StmtPtrVariant> elseBranch) noexcept;
};
StmtPtrVariant createIfSPV(ExprPtrVariant condition, StmtPtrVariant thenBranch,
                           std::optional<StmtPtrVariant> elseBranch);

struct WhileStmt final {
  ExprPtrVariant condition;
  StmtPtrVariant body;
  explicit constexpr WhileStmt(ExprPtrVariant condition,
                               StmtPtrVariant body) noexcept;
};
StmtPtrVariant createWhileSPV(ExprPtrVariant condition, StmtPtrVariant body);

struct PrintStmt final {
  ExprPtrVariant value;
  explicit constexpr PrintStmt(ExprPtrVariant value) noexcept;
};
StmtPtrVariant createPrintSPV(ExprPtrVariant value);

struct ExprStmt final {
  ExprPtrVariant expression;
  explicit constexpr ExprStmt(ExprPtrVariant expression) noexcept;
};
StmtPtrVariant createExprSPV(ExprPtrVariant expression);

struct FunctionStmt final {
  std::vector<Token> params;
  std::vector<StmtPtrVariant> body;
  explicit constexpr FunctionStmt(std::vector<Token> params,
                                  std::vector<StmtPtrVariant> body) noexcept;
};
StmtPtrVariant createFunctionSPV(std::vector<Token> params,
                                 std::vector<StmtPtrVariant> body);

struct BlockStmt final {
  std::vector<StmtPtrVariant> statements;
  explicit constexpr BlockStmt(std::vector<StmtPtrVariant> statements) noexcept;
};
StmtPtrVariant createBlockSPV(std::vector<StmtPtrVariant> statements);

struct ReturnStmt final {
  Token retToken;
  ExprPtrVariant retValue;
  bool isFunction;
  explicit constexpr ReturnStmt(Token token, ExprPtrVariant retValue,
                                bool isFunction) noexcept;
};
StmtPtrVariant createReturnSPV(Token token, ExprPtrVariant retValue,
                               bool isFunction);

struct NullStmt final {
  explicit constexpr NullStmt() noexcept;
};
StmtPtrVariant createNullSPV();

} // namespace prsl::AST