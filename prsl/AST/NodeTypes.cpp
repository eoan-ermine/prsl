#include "NodeTypes.hpp"

namespace prsl::AST {

LiteralExpr::LiteralExpr(int value) { literalVal = value; }

GroupingExpr::GroupingExpr(ExprPtrVariant expression)
    : expression(std::move(expression)) {}

VarExpr::VarExpr(Token ident) : ident(ident) {}

InputExpr::InputExpr() {}

AssignmentExpr::AssignmentExpr(Token varName, ExprPtrVariant initializer)
    : varName(varName), initializer(std::move(initializer)) {}

UnaryExpr::UnaryExpr(ExprPtrVariant expression, Types::Token op)
    : expression(std::move(expression)), op(op) {}

BinaryExpr::BinaryExpr(ExprPtrVariant lhsExpression, Types::Token op,
                       ExprPtrVariant rhsExpression)
    : lhsExpression(std::move(lhsExpression)), op(op),
      rhsExpression(std::move(rhsExpression)) {}

PostfixExpr::PostfixExpr(ExprPtrVariant expression, Types::Token op)
    : expression(std::move(expression)), op(op) {}

VarStmt::VarStmt(Token varName, ExprPtrVariant initializer)
    : varName(varName), initializer(std::move(initializer)) {}

IfStmt::IfStmt(ExprPtrVariant condition, StmtPtrVariant thenBranch,
               std::optional<StmtPtrVariant> elseBranch)
    : condition(std::move(condition)), thenBranch(std::move(thenBranch)),
      elseBranch(std::move(elseBranch)) {}

BlockStmt::BlockStmt(std::vector<StmtPtrVariant> statements)
    : statements(std::move(statements)) {}

WhileStmt::WhileStmt(ExprPtrVariant condition, StmtPtrVariant body)
    : condition(std::move(condition)), body(std::move(body)) {}

PrintStmt::PrintStmt(ExprPtrVariant value) : value(std::move(value)) {}

ExprStmt::ExprStmt(ExprPtrVariant expression)
    : expression(std::move(expression)) {}

FunctionStmt::FunctionStmt(std::vector<Token> params,
                           std::vector<StmtPtrVariant> body)
    : params(std::move(params)), body(std::move(body)) {}

ExprPtrVariant createLiteralEPV(int literalVal) {
  return std::make_unique<LiteralExpr>(literalVal);
}

ExprPtrVariant createGroupingEPV(ExprPtrVariant expression) {
  return std::make_unique<GroupingExpr>(std::move(expression));
}

ExprPtrVariant createVarEPV(Token ident) {
  return std::make_unique<VarExpr>(ident);
}

ExprPtrVariant createInputEPV() { return std::make_unique<InputExpr>(); }

ExprPtrVariant createAssignmentEPV(Token varName, ExprPtrVariant initializer) {
  return std::make_unique<AssignmentExpr>(varName, std::move(initializer));
}

ExprPtrVariant createUnaryEPV(ExprPtrVariant expression, Types::Token op) {
  return std::make_unique<UnaryExpr>(std::move(expression), op);
}

ExprPtrVariant createBinaryEPV(ExprPtrVariant lhsExpression, Types::Token op,
                               ExprPtrVariant rhsExpression) {
  return std::make_unique<BinaryExpr>(std::move(lhsExpression), op,
                                      std::move(rhsExpression));
}

ExprPtrVariant createPostfixEPV(ExprPtrVariant expression, Types::Token op) {
  return std::make_unique<PostfixExpr>(std::move(expression), op);
}

StmtPtrVariant createVarSPV(Token varName, ExprPtrVariant initializer) {
  return std::make_unique<VarStmt>(varName, std::move(initializer));
}

StmtPtrVariant createIfSPV(ExprPtrVariant condition, StmtPtrVariant thenBranch,
                           std::optional<StmtPtrVariant> elseBranch) {
  return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch),
                                  std::move(elseBranch));
}

StmtPtrVariant createBlockSPV(std::vector<StmtPtrVariant> statements) {
  return std::make_unique<BlockStmt>(std::move(statements));
}

StmtPtrVariant createWhileSPV(ExprPtrVariant condition, StmtPtrVariant body) {
  return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

StmtPtrVariant createPrintSPV(ExprPtrVariant value) {
  return std::make_unique<PrintStmt>(std::move(value));
}

StmtPtrVariant createExprSPV(ExprPtrVariant expression) {
  return std::make_unique<ExprStmt>(std::move(expression));
}

StmtPtrVariant createFunctionSPV(std::vector<Token> params,
                                 std::vector<StmtPtrVariant> body) {
  return std::make_unique<FunctionStmt>(std::move(params), std::move(body));
}

} // namespace prsl::AST