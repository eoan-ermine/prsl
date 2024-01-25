#include "Evaluator.hpp"

#include "prsl/Debug/RuntimeError.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <variant>

namespace prsl::Evaluator {

Evaluator::Evaluator(ErrorReporter &eReporter)
    : eReporter(eReporter), envManager(eReporter) {}

PrslObject Evaluator::evaluateExpr(const ExprPtrVariant &expr) {
  switch (expr.index()) {
  case 0:
    return evaluateLiteralExpr(std::get<0>(expr));
  case 1:
    return evaluateGroupingExpr(std::get<1>(expr));
  case 2:
    return evaluateVarExpr(std::get<2>(expr));
  case 3:
    return evaluateInputExpr(std::get<3>(expr));
  case 4:
    return evaluateAssignmentExpr(std::get<4>(expr));
  case 5:
    return evaluateUnaryExpr(std::get<5>(expr));
  case 6:
    return evaluateBinaryExpr(std::get<6>(expr));
  case 7:
    return evaluatePostfixExpr(std::get<7>(expr));
  default:
    std::unreachable();
  }
}

void Evaluator::evaluateStmt(const StmtPtrVariant &stmt) {
  switch (stmt.index()) {
  case 0:
    return evaluateVarStmt(std::get<0>(stmt));
  case 1:
    return evaluateIfStmt(std::get<1>(stmt));
  case 2:
    return evaluateBlockStmt(std::get<2>(stmt));
  case 3:
    return evaluateWhileStmt(std::get<3>(stmt));
  case 4:
    return evaluatePrintStmt(std::get<4>(stmt));
  case 5:
    return evaluateExprStmt(std::get<5>(stmt));
  default:
    std::unreachable();
  }
}

void Evaluator::executeStmts(const std::vector<StmtPtrVariant> &stmts) {
  for (const auto &stmt : stmts) {
    try {
      evaluateStmt(stmt);
    } catch (const Errors::RuntimeError &e) {
      eReporter.printToErr();
    }
  }
}

void Evaluator::dump(std::string_view) {
  // NO-OP
}

int Evaluator::getInt(const Token &token, const PrslObject &obj) {
  if (!std::holds_alternative<int>(obj))
    throw Errors::reportRuntimeError(
        eReporter, token,
        "Attempt to perform arithmetic operation on non-numeric literal " +
            toString(obj));
  return std::get<int>(obj);
}

PrslObject Evaluator::evaluateLiteralExpr(const LiteralExprPtr &expr) {
  return PrslObject(expr->literalVal);
}

PrslObject Evaluator::evaluateGroupingExpr(const GroupingExprPtr &expr) {
  return evaluateExpr(expr->expression);
}

PrslObject Evaluator::evaluateVarExpr(const VarExprPtr &expr) {
  return envManager.get(expr->ident);
}

PrslObject Evaluator::evaluateInputExpr(const InputExprPtr &expr) {
  int val;
  std::cin >> val;
  return PrslObject(val);
}

PrslObject Evaluator::evaluateAssignmentExpr(const AssignmentExprPtr &expr) {
  envManager.define(expr->varName, evaluateExpr(expr->initializer));
  return envManager.get(expr->varName);
}

PrslObject Evaluator::evaluateUnaryExpr(const UnaryExprPtr &expr) {
  PrslObject obj = evaluateExpr(expr->expression);

  switch (expr->op.getType()) {
  case Token::Type::MINUS:
    return -getInt(expr->op, obj);
  default:
    break;
  }

  throw Errors::reportRuntimeError(
      eReporter, expr->op,
      "Illegal unary expression: " + expr->op.toString() + toString(obj));
}

PrslObject Evaluator::evaluateBinaryExpr(const BinaryExprPtr &expr) {
  auto lhs = evaluateExpr(expr->lhsExpression);
  auto rhs = evaluateExpr(expr->rhsExpression);

  switch (expr->op.getType()) {
  case Token::Type::PLUS:
    return getInt(expr->op, lhs) + getInt(expr->op, rhs);
  case Token::Type::MINUS:
    return getInt(expr->op, lhs) - getInt(expr->op, rhs);
  case Token::Type::STAR:
    return getInt(expr->op, lhs) * getInt(expr->op, rhs);
  case Token::Type::SLASH: {
    int denominator = getInt(expr->op, rhs);
    if (denominator == 0)
      throw Errors::reportRuntimeError(eReporter, expr->op, "Division by zero");
    return getInt(expr->op, lhs) / getInt(expr->op, rhs);
  }
  case Token::Type::NOT_EQUAL:
    return !areEqual(lhs, rhs);
  case Token::Type::EQUAL_EQUAL:
    return areEqual(lhs, rhs);
  case Token::Type::LESS:
    return getInt(expr->op, lhs) < getInt(expr->op, rhs);
  case Token::Type::LESS_EQUAL:
    return getInt(expr->op, lhs) <= getInt(expr->op, rhs);
  case Token::Type::GREATER:
    return getInt(expr->op, lhs) > getInt(expr->op, rhs);
  case Token::Type::GREATER_EQUAL:
    return getInt(expr->op, lhs) >= getInt(expr->op, rhs);
  default:
    break;
  }

  throw reportRuntimeError(eReporter, expr->op,
                           "Illegal operator in expression: " + toString(lhs) +
                               expr->op.toString() + toString(rhs));
}

static PrslObject postfixExpr(ErrorReporter &eReporter, const Token &op,
                              const PrslObject &obj) {
  if (std::holds_alternative<int>(obj)) {
    int val = std::get<int>(obj);
    switch (op.getType()) {
    case Token::Type::PLUS_PLUS:
      return PrslObject(val + 1);
    case Token::Type::MINUS_MINUS:
      return PrslObject(val - 1);
    default:
      break;
    }
  }

  throw reportRuntimeError(eReporter, op,
                           "Illegal operator in expression: " + op.toString() +
                               toString(obj));
}

PrslObject Evaluator::evaluatePostfixExpr(const PostfixExprPtr &expr) {
  PrslObject obj = evaluateExpr(expr->expression);
  if (std::holds_alternative<VarExprPtr>(expr->expression)) {
    envManager.assign(std::get<VarExprPtr>(expr->expression)->ident,
                      postfixExpr(eReporter, expr->op, obj));
  }
  return obj;
}

void Evaluator::evaluateVarStmt(const VarStmtPtr &stmt) {
  envManager.define(stmt->varName, evaluateExpr(stmt->initializer));
}

void Evaluator::evaluateIfStmt(const IfStmtPtr &stmt) {
  if (isTrue(evaluateExpr(stmt->condition)))
    return evaluateStmt(stmt->thenBranch);
  if (stmt->elseBranch.has_value())
    return evaluateStmt(stmt->elseBranch.value());
}

void Evaluator::evaluateBlockStmt(const BlockStmtPtr &stmt) {
  auto curEnv = envManager.getCurEnv();
  envManager.createNewEnv();
  executeStmts(stmt->statements);
  envManager.discardEnvsTill(curEnv);
}

void Evaluator::evaluateWhileStmt(const WhileStmtPtr &stmt) {
  while (isTrue(evaluateExpr(stmt->condition)))
    evaluateStmt(stmt->body);
}

void Evaluator::evaluatePrintStmt(const PrintStmtPtr &stmt) {
  auto obj = evaluateExpr(stmt->value);
  std::cout << toString(obj) << std::endl;
}

void Evaluator::evaluateExprStmt(const ExprStmtPtr &stmt) {
  evaluateExpr(stmt->expression);
}

} // namespace prsl::Evaluator