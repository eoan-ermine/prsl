#pragma once

#include <exception>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "prsl/AST/NodeTypes.hpp"
#include "prsl/Debug/ErrorReporter.hpp"
#include "prsl/Debug/RuntimError.hpp"
#include "prsl/Evaluator/Environment.hpp"
#include "prsl/Evaluator/Objects.hpp"
#include "prsl/Types/Token.hpp"

namespace prsl::Evaluator {

using namespace AST;

using Errors::ErrorReporter;

using Types::Token;
using Type = Types::Token::Type;

class Evaluator {
public:
  explicit Evaluator(ErrorReporter &eReporter)
      : eReporter(eReporter), envManager(eReporter) {}

  auto evaluateExpr(const ExprPtrVariant &expr) -> PrslObject {
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

  void evaluateStmt(const StmtPtrVariant &stmt) {
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
    default:
      std::unreachable();
    }
  }

  void evaluateStmts(const std::vector<StmtPtrVariant> &stmts) {
    for (const auto &stmt : stmts) {
      try {
        evaluateStmt(stmt);
      } catch (const Errors::RuntimeError &e) {
        eReporter.printToErr();
      }
    }
  }

private:
  auto getInt(const Token &token, const PrslObject &obj) -> int {
    if (!std::holds_alternative<int>(obj))
      throw Errors::reportRuntimeError(
          eReporter, token,
          "Attempt to perform arithmetic operation on non-numeric literal " +
              toString(obj));
    return std::get<int>(obj);
  }

  auto evaluateLiteralExpr(const LiteralExprPtr &expr) -> PrslObject {
    return PrslObject(expr->literalVal);
  }

  auto evaluateGroupingExpr(const GroupingExprPtr &expr) -> PrslObject {
    return evaluateExpr(expr->expression);
  }

  auto evaluateVarExpr(const VarExprPtr &expr) -> PrslObject {
    return envManager.get(expr->ident);
  }

  auto evaluateInputExpr(const InputExprPtr &expr) -> PrslObject {
    int val;
    std::cin >> val;
    return PrslObject(val);
  }

  auto evaluateAssignmentExpr(const AssignmentExprPtr &expr) -> PrslObject {
    envManager.assign(expr->varName, evaluateExpr(expr->initializer));
    return envManager.get(expr->varName);
  }

  auto evaluateUnaryExpr(const UnaryExprPtr &expr) -> PrslObject {
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

  auto evaluateBinaryExpr(const BinaryExprPtr &expr) -> PrslObject {
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
        throw Errors::reportRuntimeError(eReporter, expr->op,
                                         "Division by zero");
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

    throw reportRuntimeError(
        eReporter, expr->op,
        "Illegal operator in expression: " + toString(lhs) +
            expr->op.toString() + toString(rhs));
  }

  auto postfixExpr(const Token &op, const PrslObject &obj) -> PrslObject {
    if (std::holds_alternative<int>(obj)) {
      int val = std::get<int>(obj);
      switch (op.getType()) {
      case Token::Type::PLUS_PLUS:
        return PrslObject(++val);
      case Token::Type::MINUS_MINUS:
        return PrslObject(--val);
      default:
        break;
      }
    }

    throw reportRuntimeError(
        eReporter, op,
        "Illegal operator in expression: " + op.toString() + toString(obj));
  }

  auto evaluatePostfixExpr(const PostfixExprPtr &expr) -> PrslObject {
    PrslObject obj = evaluateExpr(expr->expression);
    if (std::holds_alternative<VarExprPtr>(expr->expression)) {
      envManager.assign(std::get<VarExprPtr>(expr->expression)->ident,
                        postfixExpr(expr->op, obj));
    }
    return obj;
  }

  void evaluateVarStmt(const VarStmtPtr &stmt) {
    envManager.define(stmt->varName, evaluateExpr(stmt->initializer));
  }

  void evaluateIfStmt(const IfStmtPtr &stmt) {
    if (isTrue(evaluateExpr(stmt->condition)))
      return evaluateStmt(stmt->thenBranch);
    if (stmt->elseBranch.has_value())
      return evaluateStmt(stmt->elseBranch.value());
  }

  void evaluateBlockStmt(const BlockStmtPtr &stmt) {
    auto curEnv = envManager.getCurEnv();
    envManager.createNewEnviron();
    evaluateStmts(stmt->statements);
    envManager.discardEnvironsTill(curEnv);
  }

  void evaluateWhileStmt(const WhileStmtPtr &stmt) {
    while (isTrue(evaluateExpr(stmt->condition)))
      evaluateStmt(stmt->body);
  }

  void evaluatePrintStmt(const PrintStmtPtr &stmt) {
    auto obj = evaluateExpr(stmt->value);
    std::cout << toString(obj) << std::endl;
  }

  ErrorReporter &eReporter;
  EnvironmentManager envManager;
};

} // namespace prsl::Evaluator