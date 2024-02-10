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

struct ScopeExpr;

struct FuncExpr;

struct CallExpr;

using LiteralExprPtr = std::unique_ptr<LiteralExpr>;

using GroupingExprPtr = std::unique_ptr<GroupingExpr>;

using VarExprPtr = std::unique_ptr<VarExpr>;

using InputExprPtr = std::unique_ptr<InputExpr>;

using AssignmentExprPtr = std::unique_ptr<AssignmentExpr>;

using UnaryExprPtr = std::unique_ptr<UnaryExpr>;

using BinaryExprPtr = std::unique_ptr<BinaryExpr>;

using PostfixExprPtr = std::unique_ptr<PostfixExpr>;

using ScopeExprPtr = std::unique_ptr<ScopeExpr>;

using FuncExprPtr = std::unique_ptr<FuncExpr>;

using CallExprPtr = std::unique_ptr<CallExpr>;

using ExprPtrVariant =
    std::variant<LiteralExprPtr, GroupingExprPtr, VarExprPtr, InputExprPtr,
                 AssignmentExprPtr, UnaryExprPtr, BinaryExprPtr, PostfixExprPtr,
                 ScopeExprPtr, FuncExprPtr, CallExprPtr>;

struct VarStmt;

struct IfStmt;

struct WhileStmt;

struct PrintStmt;

struct ExprStmt;

struct FunctionStmt;

struct BlockStmt;

struct ReturnStmt;

struct NullStmt;

using VarStmtPtr = std::unique_ptr<VarStmt>;

using IfStmtPtr = std::unique_ptr<IfStmt>;

using WhileStmtPtr = std::unique_ptr<WhileStmt>;

using PrintStmtPtr = std::unique_ptr<PrintStmt>;

using ExprStmtPtr = std::unique_ptr<ExprStmt>;

using FunctionStmtPtr = std::unique_ptr<FunctionStmt>;

using BlockStmtPtr = std::unique_ptr<BlockStmt>;

using ReturnStmtPtr = std::unique_ptr<ReturnStmt>;

using NullStmtPtr = std::unique_ptr<NullStmt>;

using StmtPtrVariant =
    std::variant<VarStmtPtr, IfStmtPtr, WhileStmtPtr, PrintStmtPtr, ExprStmtPtr,
                 FunctionStmtPtr, BlockStmtPtr, ReturnStmtPtr, NullStmtPtr>;

using prsl::Types::Token;

struct LiteralExpr final {
  int literalVal;
  explicit LiteralExpr(int value);
};

struct GroupingExpr final {
  ExprPtrVariant expression;
  explicit GroupingExpr(ExprPtrVariant expression);
};

struct VarExpr final {
  Token ident;
  explicit VarExpr(Token ident);
};

struct InputExpr final {
  explicit InputExpr();
};

struct AssignmentExpr final {
  Token varName;
  ExprPtrVariant initializer;
  explicit AssignmentExpr(Token varName, ExprPtrVariant initializer);
};

struct UnaryExpr final {
  Types::Token op;
  ExprPtrVariant expression;
  explicit UnaryExpr(ExprPtrVariant expression, Types::Token op);
};

struct BinaryExpr final {
  Types::Token op;
  ExprPtrVariant lhsExpression;
  ExprPtrVariant rhsExpression;
  explicit BinaryExpr(ExprPtrVariant lhsExpression, Types::Token op,
                      ExprPtrVariant rhsExpression);
};

struct PostfixExpr final {
  Types::Token op;
  ExprPtrVariant expression;
  explicit PostfixExpr(ExprPtrVariant expression, Types::Token op);
};

struct ScopeExpr final {
  std::vector<StmtPtrVariant> statements;
  explicit ScopeExpr(std::vector<StmtPtrVariant> statements);
};

struct FuncExpr final {
  // For rerurning errors
  Token token;
  std::optional<Token> name;
  std::vector<Token> parameters;
  ExprPtrVariant body;
  std::optional<ExprPtrVariant> retExpr;
  FuncExpr(Token token, std::optional<Token> name,
           std::vector<Token> parameters);
};

struct CallExpr final {
  Token ident;
  std::vector<ExprPtrVariant> arguments;
  explicit CallExpr(Token ident, std::vector<ExprPtrVariant> arguments);
};

struct VarStmt final {
  Token varName;
  ExprPtrVariant initializer;
  explicit VarStmt(Token varName, ExprPtrVariant initializer);
};

struct IfStmt final {
  ExprPtrVariant condition;
  StmtPtrVariant thenBranch;
  std::optional<StmtPtrVariant> elseBranch;

  explicit IfStmt(ExprPtrVariant condition, StmtPtrVariant thenBranch,
                  std::optional<StmtPtrVariant> elseBranch);
};

struct WhileStmt final {
  ExprPtrVariant condition;
  StmtPtrVariant body;
  explicit WhileStmt(ExprPtrVariant condition, StmtPtrVariant body);
};

struct PrintStmt final {
  ExprPtrVariant value;
  explicit PrintStmt(ExprPtrVariant value);
};

struct ExprStmt final {
  ExprPtrVariant expression;
  explicit ExprStmt(ExprPtrVariant expression);
};

struct FunctionStmt final {
  std::vector<Token> params;
  std::vector<StmtPtrVariant> body;
  explicit FunctionStmt(std::vector<Token> params,
                        std::vector<StmtPtrVariant> body);
};

struct BlockStmt final {
  std::vector<StmtPtrVariant> statements;
  explicit BlockStmt(std::vector<StmtPtrVariant> statements);
};

struct ReturnStmt final {
  Token retToken;
  ExprPtrVariant retValue;
  bool isFunction;
  explicit ReturnStmt(Token token, ExprPtrVariant retValue, bool isFunction);
};

struct NullStmt final {
  explicit NullStmt();
};

ExprPtrVariant createLiteralEPV(int literalVal);

ExprPtrVariant createGroupingEPV(ExprPtrVariant expression);

ExprPtrVariant createVarEPV(Token ident);

ExprPtrVariant createInputEPV();

ExprPtrVariant createAssignmentEPV(Token varName, ExprPtrVariant initializer);

ExprPtrVariant createUnaryEPV(ExprPtrVariant expression, Types::Token op);

ExprPtrVariant createBinaryEPV(ExprPtrVariant lhsExpression, Types::Token op,
                               ExprPtrVariant rhsExpression);

ExprPtrVariant createPostfixEPV(ExprPtrVariant expression, Types::Token op);

ExprPtrVariant createScopeEPV(std::vector<StmtPtrVariant> statements);

ExprPtrVariant createFuncEPV(Token token, std::optional<Token> name,
                             std::vector<Token> parameters);

ExprPtrVariant createCallEPV(Token ident,
                             std::vector<ExprPtrVariant> arguments);

StmtPtrVariant createVarSPV(Token varName, ExprPtrVariant initializer);

StmtPtrVariant createIfSPV(ExprPtrVariant condition, StmtPtrVariant thenBranch,
                           std::optional<StmtPtrVariant> elseBranch);

StmtPtrVariant createWhileSPV(ExprPtrVariant condition, StmtPtrVariant body);

StmtPtrVariant createPrintSPV(ExprPtrVariant value);

StmtPtrVariant createExprSPV(ExprPtrVariant expression);

StmtPtrVariant createFunctionSPV(std::vector<Token> params,
                                 std::vector<StmtPtrVariant> body);

StmtPtrVariant createBlockSPV(std::vector<StmtPtrVariant> statements);

StmtPtrVariant createReturnSPV(Token token, ExprPtrVariant retValue,
                               bool isFunction);

StmtPtrVariant createNullSPV();

} // namespace prsl::AST