#include "Evaluator.hpp"

#include "prsl/AST/TreeWalkerVisitor.hpp"
#include "prsl/Debug/RuntimeError.hpp"
#include <iostream>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

namespace prsl::Evaluator {

Evaluator::Evaluator(ErrorReporter &eReporter)
    : eReporter(eReporter), envManager(eReporter) {}

bool Evaluator::dump(std::string_view) { return false; }

int Evaluator::getInt(const Token &token, const PrslObject &obj) {
  if (!std::holds_alternative<int>(obj))
    throw Errors::reportRuntimeError(
        eReporter, token,
        "Attempt to perform arithmetic operation on non-numeric literal " +
            toString(obj));
  return std::get<int>(obj);
}

PrslObject Evaluator::visitLiteralExpr(const LiteralExprPtr &expr) {
  return PrslObject(expr->literalVal);
}

PrslObject Evaluator::visitGroupingExpr(const GroupingExprPtr &expr) {
  return visitExpr(expr->expression);
}

PrslObject Evaluator::visitVarExpr(const VarExprPtr &expr) {
  return envManager.get(expr->ident);
}

PrslObject Evaluator::visitInputExpr(const InputExprPtr &expr) {
  int val;
  std::cin >> val;
  return PrslObject(val);
}

PrslObject Evaluator::visitAssignmentExpr(const AssignmentExprPtr &expr) {
  envManager.define(expr->varName, visitExpr(expr->initializer));
  return envManager.get(expr->varName);
}

PrslObject Evaluator::visitUnaryExpr(const UnaryExprPtr &expr) {
  PrslObject obj = visitExpr(expr->expression);

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

PrslObject Evaluator::visitBinaryExpr(const BinaryExprPtr &expr) {
  auto lhs = visitExpr(expr->lhsExpression);
  auto rhs = visitExpr(expr->rhsExpression);

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

PrslObject Evaluator::visitPostfixExpr(const PostfixExprPtr &expr) {
  PrslObject obj = visitExpr(expr->expression);
  if (std::holds_alternative<VarExprPtr>(expr->expression)) {
    envManager.assign(std::get<VarExprPtr>(expr->expression)->ident,
                      postfixExpr(eReporter, expr->op, obj));
  }
  return obj;
}

PrslObject Evaluator::evaluateScope(const ScopeExprPtr &stmt) {
  PrslObject res{0};
  for (const auto &stmt :
       stmt->statements | std::views::take(stmt->statements.size() - 1)) {
    visitStmt(stmt);
  }
  if (stmt->statements.size()) {
    const auto &back = stmt->statements.back();
    if (std::holds_alternative<ExprStmtPtr>(back)) {
      res = visitExpr(std::get<ExprStmtPtr>(back)->expression);
    } else {
      visitStmt(back);
    }
  }
  return res;
}

PrslObject Evaluator::visitScopeExpr(const ScopeExprPtr &stmt) {
  PrslObject res;
  envManager.withNewEnviron([&] { res = evaluateScope(stmt); });
  return res;
}

PrslObject Evaluator::visitFuncExpr(const FuncExprPtr &expr) {
  auto obj = PrslObject{std::make_shared<FuncObj>(expr)};

  class FunctionsResolver : public TreeWalkerVisitor {
  public:
    FunctionsResolver(
        std::unordered_map<std::string_view, PrslObject> &functionsManager)
        : functionsManager(functionsManager) {}
    bool dump(std::string_view) override { return false; }
    void visitFuncExpr(const FuncExprPtr &expr) override {
      functionsManager.emplace(expr->name->getLexeme(),
                               PrslObject{std::make_shared<FuncObj>(expr)});
    }

  private:
    std::unordered_map<std::string_view, PrslObject> &functionsManager;
  };
  FunctionsResolver funcResolver(functionsManager);
  funcResolver.visitExpr(expr->body);

  if (expr->name) {
    functionsManager.emplace(expr->name->getLexeme(), obj);
  }

  return obj;
}

PrslObject Evaluator::visitCallExpr(const CallExprPtr &expr) {
  PrslObject res{0};
  PrslObject obj = nullptr;

  // Initialize obj, it can be in functionsManager or envManager
  if (functionsManager.contains(expr->ident.getLexeme())) {
    obj = functionsManager[expr->ident.getLexeme()];
  }
  if (std::holds_alternative<std::nullptr_t>(obj) &&
      envManager.contains(expr->ident)) {
    obj = envManager.get(expr->ident);
    if (!std::holds_alternative<FuncObjPtr>(obj))
      throw reportRuntimeError(eReporter, expr->ident, "Not a function");
  }

  auto func = std::get<FuncObjPtr>(obj);

  // Check parameters count
  if (size_t paramsCount = func->paramsCount(),
      argsCount = expr->arguments.size();
      paramsCount != argsCount) {
    throw reportRuntimeError(eReporter, expr->ident,
                             "Wrong number of arguments");
  }

  // Evaluate arguments
  std::vector<PrslObject> args;
  for (const auto &arg : expr->arguments) {
    args.push_back(visitExpr(arg));
  }

  auto funcEnv = std::make_shared<Environment<PrslObject>>(nullptr);
  envManager.withNewEnviron(funcEnv, [&] {
    const auto &params = func->getDeclaration()->parameters;
    auto paramIt = params.begin();
    auto argIt = args.begin();
    for (; paramIt != params.end() && argIt != args.end(); ++paramIt, ++argIt) {
      envManager.define(*paramIt, *argIt);
    }

    auto scopeRes =
        evaluateScope(std::get<ScopeExprPtr>(func->getDeclaration()->body));
    if (func->getDeclaration()->retExpr) {
      res = visitExpr(*func->getDeclaration()->retExpr);
    } else {
      res = std::move(scopeRes);
    }

    if (!std::holds_alternative<int>(res)) {
      throw reportRuntimeError(eReporter, func->getDeclaration()->token,
                               "Wrong return value");
    }
  });

  return res;
}

void Evaluator::visitVarStmt(const VarStmtPtr &stmt) {
  envManager.define(stmt->varName, visitExpr(stmt->initializer));
}

void Evaluator::visitIfStmt(const IfStmtPtr &stmt) {
  if (isTrue(visitExpr(stmt->condition)))
    return visitStmt(stmt->thenBranch);
  if (stmt->elseBranch.has_value())
    return visitStmt(stmt->elseBranch.value());
}

void Evaluator::visitWhileStmt(const WhileStmtPtr &stmt) {
  while (isTrue(visitExpr(stmt->condition)))
    visitStmt(stmt->body);
}

void Evaluator::visitPrintStmt(const PrintStmtPtr &stmt) {
  auto obj = visitExpr(stmt->value);
  std::cout << toString(obj) << std::endl;
}

void Evaluator::visitExprStmt(const ExprStmtPtr &stmt) {
  visitExpr(stmt->expression);
}

void Evaluator::visitFunctionStmt(const FunctionStmtPtr &stmt) {
  for (const auto &stmt : stmt->body) {
    visitStmt(stmt);
  }
}

void Evaluator::visitBlockStmt(const BlockStmtPtr &stmt) {
  envManager.withNewEnviron([&] {
    for (const auto &stmt : stmt->statements) {
      visitStmt(stmt);
    }
  });
}

void Evaluator::visitReturnStmt(const ReturnStmtPtr &stmt) {}

} // namespace prsl::Evaluator