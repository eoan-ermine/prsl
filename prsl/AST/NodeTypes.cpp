#include "prsl/AST/NodeTypes.hpp"

namespace prsl::AST {

constexpr LiteralExpr::LiteralExpr(int value) noexcept { literalVal = value; }

ExprPtrVariant createLiteralEPV(int literalVal) {
  return std::make_unique<LiteralExpr>(literalVal);
}

constexpr GroupingExpr::GroupingExpr(ExprPtrVariant expression) noexcept
    : expression(std::move(expression)) {}

ExprPtrVariant createGroupingEPV(ExprPtrVariant expression) {
  return std::make_unique<GroupingExpr>(std::move(expression));
}

constexpr VarExpr::VarExpr(Token ident) noexcept : ident(ident) {}

ExprPtrVariant createVarEPV(Token ident) {
  return std::make_unique<VarExpr>(ident);
}

constexpr InputExpr::InputExpr() noexcept {}

ExprPtrVariant createInputEPV() { return std::make_unique<InputExpr>(); }

constexpr AssignmentExpr::AssignmentExpr(Token varName,
                                         ExprPtrVariant initializer) noexcept
    : varName(varName), initializer(std::move(initializer)) {}

ExprPtrVariant createAssignmentEPV(Token varName, ExprPtrVariant initializer) {
  return std::make_unique<AssignmentExpr>(varName, std::move(initializer));
}

constexpr UnaryExpr::UnaryExpr(ExprPtrVariant expression,
                               Types::Token op) noexcept
    : expression(std::move(expression)), op(op) {}

ExprPtrVariant createUnaryEPV(ExprPtrVariant expression, Types::Token op) {
  return std::make_unique<UnaryExpr>(std::move(expression), op);
}

constexpr BinaryExpr::BinaryExpr(ExprPtrVariant lhsExpression, Types::Token op,
                                 ExprPtrVariant rhsExpression) noexcept
    : lhsExpression(std::move(lhsExpression)), op(op),
      rhsExpression(std::move(rhsExpression)) {}

ExprPtrVariant createBinaryEPV(ExprPtrVariant lhsExpression, Types::Token op,
                               ExprPtrVariant rhsExpression) {
  return std::make_unique<BinaryExpr>(std::move(lhsExpression), op,
                                      std::move(rhsExpression));
}

constexpr PostfixExpr::PostfixExpr(ExprPtrVariant expression,
                                   Types::Token op) noexcept
    : expression(std::move(expression)), op(op) {}

ExprPtrVariant createPostfixEPV(ExprPtrVariant expression, Types::Token op) {
  return std::make_unique<PostfixExpr>(std::move(expression), op);
}

constexpr ScopeExpr::ScopeExpr(std::vector<StmtPtrVariant> statements) noexcept
    : statements(std::move(statements)) {}

ExprPtrVariant createScopeEPV(std::vector<StmtPtrVariant> statements) {
  return std::make_unique<ScopeExpr>(std::move(statements));
}

constexpr FuncExpr::FuncExpr(Token token, std::optional<Token> name,
                             std::vector<Token> parameters) noexcept
    : token(std::move(token)), name(std::move(name)),
      parameters(std::move(parameters)) {}

ExprPtrVariant createFuncEPV(Token token, std::optional<Token> name,
                             std::vector<Token> parameters) {
  return std::make_unique<FuncExpr>(std::move(token), std::move(name),
                                    std::move(parameters));
}

constexpr CallExpr::CallExpr(Token ident,
                             std::vector<ExprPtrVariant> arguments) noexcept
    : ident(std::move(ident)), arguments(std::move(arguments)) {}

ExprPtrVariant createCallEPV(Token ident,
                             std::vector<ExprPtrVariant> arguments) {
  return std::make_unique<CallExpr>(std::move(ident), std::move(arguments));
}

constexpr VarStmt::VarStmt(Token varName, ExprPtrVariant initializer) noexcept
    : varName(varName), initializer(std::move(initializer)) {}

StmtPtrVariant createVarSPV(Token varName, ExprPtrVariant initializer) {
  return std::make_unique<VarStmt>(varName, std::move(initializer));
}

constexpr IfStmt::IfStmt(ExprPtrVariant condition, StmtPtrVariant thenBranch,
                         std::optional<StmtPtrVariant> elseBranch) noexcept
    : condition(std::move(condition)), thenBranch(std::move(thenBranch)),
      elseBranch(std::move(elseBranch)) {}

StmtPtrVariant createIfSPV(ExprPtrVariant condition, StmtPtrVariant thenBranch,
                           std::optional<StmtPtrVariant> elseBranch) {
  return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch),
                                  std::move(elseBranch));
}

constexpr WhileStmt::WhileStmt(ExprPtrVariant condition,
                               StmtPtrVariant body) noexcept
    : condition(std::move(condition)), body(std::move(body)) {}

StmtPtrVariant createWhileSPV(ExprPtrVariant condition, StmtPtrVariant body) {
  return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

constexpr PrintStmt::PrintStmt(ExprPtrVariant value) noexcept
    : value(std::move(value)) {}

StmtPtrVariant createPrintSPV(ExprPtrVariant value) {
  return std::make_unique<PrintStmt>(std::move(value));
}

constexpr ExprStmt::ExprStmt(ExprPtrVariant expression) noexcept
    : expression(std::move(expression)) {}

StmtPtrVariant createExprSPV(ExprPtrVariant expression) {
  return std::make_unique<ExprStmt>(std::move(expression));
}

constexpr FunctionStmt::FunctionStmt(std::vector<Token> params,
                                     std::vector<StmtPtrVariant> body) noexcept
    : params(std::move(params)), body(std::move(body)) {}

StmtPtrVariant createFunctionSPV(std::vector<Token> params,
                                 std::vector<StmtPtrVariant> body) {
  return std::make_unique<FunctionStmt>(std::move(params), std::move(body));
}

constexpr BlockStmt::BlockStmt(std::vector<StmtPtrVariant> statements) noexcept
    : statements(std::move(statements)) {}

StmtPtrVariant createBlockSPV(std::vector<StmtPtrVariant> statements) {
  return std::make_unique<BlockStmt>(std::move(statements));
}

constexpr ReturnStmt::ReturnStmt(Token token, ExprPtrVariant retValue,
                                 bool isFunction) noexcept
    : retToken(std::move(token)), retValue(std::move(retValue)),
      isFunction(isFunction) {}

StmtPtrVariant createReturnSPV(Token token, ExprPtrVariant retValue,
                               bool isFunction) {
  return std::make_unique<ReturnStmt>(std::move(token), std::move(retValue),
                                      isFunction);
}

constexpr NullStmt::NullStmt() noexcept {}

StmtPtrVariant createNullSPV() { return std::make_unique<NullStmt>(); }

} // namespace prsl::AST