#include "prsl/Compiler/Interpreter/Interpreter.hpp"
#include "prsl/AST/TreeWalkerVisitor.hpp"
#include "prsl/Compiler/CompilerFlags.hpp"
#include "prsl/Debug/Errors.hpp"

#include <iostream>

namespace prsl::Interpreter {

Interpreter::Interpreter(Compiler::CompilerFlags *flags,
                         Logger &logger)
    : flags(flags), logger(logger), envManager(logger) {}

bool Interpreter::dump(const std::filesystem::path &path) const {
  return false;
}

int Interpreter::getInt(const Token &token, const PrslObject &obj) const {
  if (!std::holds_alternative<int>(obj))
    throw Errors::reportRuntimeError(
        logger, token,
        "Attempt to perform arithmetic operation on non-numeric literal " +
            toString(obj));
  return std::get<int>(obj);
}

PrslObject Interpreter::visitLiteralExpr(const LiteralExprPtr &expr) {
  return PrslObject(expr->literalVal);
}

PrslObject Interpreter::visitGroupingExpr(const GroupingExprPtr &expr) {
  return visitExpr(expr->expression);
}

PrslObject Interpreter::visitVarExpr(const VarExprPtr &expr) {
  return envManager.get(expr->ident);
}

PrslObject Interpreter::visitInputExpr(const InputExprPtr &expr) {
  int val;
  std::cin >> val;
  return PrslObject(val);
}

PrslObject Interpreter::visitAssignmentExpr(const AssignmentExprPtr &expr) {
  envManager.define(expr->varName, visitExpr(expr->initializer));
  return envManager.get(expr->varName);
}

PrslObject Interpreter::visitUnaryExpr(const UnaryExprPtr &expr) {
  PrslObject obj = visitExpr(expr->expression);

  switch (expr->op.getType()) {
  case Token::Type::MINUS:
    return -getInt(expr->op, obj);
  default:
    break;
  }

  throw Errors::reportRuntimeError(
      logger, expr->op,
      "Illegal unary expression: " + expr->op.toString() + toString(obj));
}

PrslObject Interpreter::visitBinaryExpr(const BinaryExprPtr &expr) {
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
      throw Errors::reportRuntimeError(logger, expr->op, "Division by zero");
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

  throw reportRuntimeError(logger, expr->op,
                           "Illegal operator in expression: " + toString(lhs) +
                               expr->op.toString() + toString(rhs));
}

static PrslObject postfixExpr(Logger &logger, const Token &op,
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

  throw reportRuntimeError(logger, op,
                           "Illegal operator in expression: " + op.toString() +
                               toString(obj));
}

PrslObject Interpreter::visitPostfixExpr(const PostfixExprPtr &expr) {
  PrslObject obj = visitExpr(expr->expression);
  if (std::holds_alternative<VarExprPtr>(expr->expression)) {
    envManager.assign(std::get<VarExprPtr>(expr->expression)->ident,
                      postfixExpr(logger, expr->op, obj));
  }
  return obj;
}

PrslObject Interpreter::evaluateScope(const ScopeExprPtr &scope) {
  for (const auto &stmt : scope->statements) {
    visitStmt(stmt);
    if (returnStack.size()) {
      auto returnValue = std::move(returnStack.top());
      returnStack.pop();
      return returnValue;
    }
  }
  return PrslObject{nullptr};
}

PrslObject Interpreter::visitScopeExpr(const ScopeExprPtr &expr) {
  PrslObject res;
  envManager.withNewEnviron([&] { res = evaluateScope(expr); });
  return res;
}

PrslObject Interpreter::visitFuncExpr(const FuncExprPtr &expr) {
  auto obj = PrslObject{std::make_shared<FuncObj>(expr)};

  class FunctionsResolver : public TreeWalkerVisitor {
  public:
    explicit FunctionsResolver(
        Types::FunctionsManager<PrslObject> &functionsManager)
        : functionsManager(functionsManager) {}
    bool dump(const std::filesystem::path &path) const { return false; }
    void visitFuncExpr(const FuncExprPtr &expr) override {
      functionsManager.set(expr->name->getLexeme(),
                           PrslObject{std::make_shared<FuncObj>(expr)});
    }

  private:
    Types::FunctionsManager<PrslObject> &functionsManager;
  };
  FunctionsResolver funcResolver(functionsManager);
  funcResolver.visitExpr(expr->body);

  if (expr->name) {
    functionsManager.set(expr->name->getLexeme(), obj);
  }

  return obj;
}

PrslObject Interpreter::visitCallExpr(const CallExprPtr &expr) {
  PrslObject obj = nullptr;

  // Initialize obj, it can be in functionsManager or envManager
  if (functionsManager.contains(expr->ident.getLexeme())) {
    obj = functionsManager.get(expr->ident.getLexeme());
  }
  if (std::holds_alternative<std::nullptr_t>(obj) &&
      envManager.contains(expr->ident)) {
    obj = envManager.get(expr->ident);
    if (!std::holds_alternative<FuncObjPtr>(obj))
      throw reportRuntimeError(logger, expr->ident, "Not a function");
  }

  auto func = std::get<FuncObjPtr>(obj);

  // Check parameters count
  if (size_t paramsCount = func->paramsCount(),
      argsCount = expr->arguments.size();
      paramsCount != argsCount) {
    throw reportRuntimeError(logger, expr->ident,
                             "Wrong number of arguments");
  }

  // Evaluate arguments
  std::vector<PrslObject> args;
  for (const auto &arg : expr->arguments) {
    args.emplace_back(visitExpr(arg));
  }

  auto funcEnv = std::make_shared<Types::Environment<PrslObject>>(nullptr);
  PrslObject res{nullptr};
  envManager.withNewEnviron(funcEnv, [&] {
    const auto &params = func->getDeclaration()->parameters;
    auto paramIt = params.begin();
    auto argIt = args.begin();
    for (; paramIt != params.end() && argIt != args.end(); ++paramIt, ++argIt) {
      envManager.define(*paramIt, *argIt);
    }

    auto scopeRes =
        evaluateScope(std::get<ScopeExprPtr>(func->getDeclaration()->body));
    res = std::move(scopeRes);
  });

  return res;
}

void Interpreter::visitVarStmt(const VarStmtPtr &stmt) {
  envManager.define(stmt->varName, visitExpr(stmt->initializer));
}

void Interpreter::visitIfStmt(const IfStmtPtr &stmt) {
  if (isTrue(visitExpr(stmt->condition)))
    return visitStmt(stmt->thenBranch);
  if (stmt->elseBranch.has_value())
    return visitStmt(stmt->elseBranch.value());
}

void Interpreter::visitWhileStmt(const WhileStmtPtr &stmt) {
  while (isTrue(visitExpr(stmt->condition)))
    visitStmt(stmt->body);
}

void Interpreter::visitPrintStmt(const PrintStmtPtr &stmt) {
  auto obj = visitExpr(stmt->value);
  std::cout << toString(obj) << std::endl;
}

void Interpreter::visitExprStmt(const ExprStmtPtr &stmt) {
  std::ignore = visitExpr(stmt->expression); // We don't need the result
}

void Interpreter::visitFunctionStmt(const FunctionStmtPtr &stmt) {
  for (const auto &stmt : stmt->body) {
    visitStmt(stmt);
  }
}

void Interpreter::visitBlockStmt(const BlockStmtPtr &stmt) {
  envManager.withNewEnviron([&] {
    for (const auto &stmt : stmt->statements) {
      visitStmt(stmt);
    }
  });
}

void Interpreter::visitReturnStmt(const ReturnStmtPtr &stmt) {
  returnStack.push(visitExpr(stmt->retValue));
}

void Interpreter::visitNullStmt(const NullStmtPtr &stmt) { return; }

} // namespace prsl::Interpreter