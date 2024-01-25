#include "prsl/AST/NodeTypes.hpp"
#include "prsl/Debug/ErrorReporter.hpp"
#include "prsl/Debug/RuntimeError.hpp"
#include "prsl/Evaluator/Environment.hpp"
#include "prsl/Types/Token.hpp"
#include <utility>

namespace prsl::Resolver {

using namespace AST;

using Errors::ErrorReporter;

using Types::Token;
using Type = Types::Token::Type;

class Resolver {
public:
  explicit Resolver(ErrorReporter &eReporter)
      : eReporter(eReporter), envManager(eReporter) {}

  void resolveExpr(const ExprPtrVariant &expr) {
    switch (expr.index()) {
    case 0:
      return resolveLiteralExpr(std::get<0>(expr));
    case 1:
      return resolveGroupingExpr(std::get<1>(expr));
    case 2:
      return resolveVarExpr(std::get<2>(expr));
    case 3:
      return resolveInputExpr(std::get<3>(expr));
    case 4:
      return resolveAssignmentExpr(std::get<4>(expr));
    case 5:
      return resolveUnaryExpr(std::get<5>(expr));
    case 6:
      return resolveBinaryExpr(std::get<6>(expr));
    case 7:
      return resolvePostfixExpr(std::get<7>(expr));
    default:
      std::unreachable();
    }
  }

  void resolveStmt(const StmtPtrVariant &stmt) {
    switch (stmt.index()) {
    case 0:
      return resolveVarStmt(std::get<0>(stmt));
    case 1:
      return resolveIfStmt(std::get<1>(stmt));
    case 2:
      return resolveBlockStmt(std::get<2>(stmt));
    case 3:
      return resolveWhileStmt(std::get<3>(stmt));
    case 4:
      return resolvePrintStmt(std::get<4>(stmt));
    case 5:
      return resolveExprStmt(std::get<5>(stmt));
    default:
      std::unreachable();
    }
  }

  void executeStmts(const std::vector<StmtPtrVariant> &stmts) {
    for (const auto &stmt : stmts) {
      try {
        resolveStmt(stmt);
      } catch (const Errors::RuntimeError &e) {
        break;
      }
    }
  }

  void dump(std::string_view filename) {}

private:
  void resolveLiteralExpr(const LiteralExprPtr &expr) { return; }

  void resolveGroupingExpr(const GroupingExprPtr &expr) {
    resolveExpr(expr->expression);
  }

  void resolveVarExpr(const VarExprPtr &expr) {
    if (!envManager.contains(expr->ident))
      throw reportRuntimeError(eReporter, expr->ident,
                               "Attempt to access an undef variable");
    if (envManager.get(expr->ident) == false) {
      throw reportRuntimeError(eReporter, expr->ident,
                               "Can't read variable in its own initializer");
    }
  }

  void resolveInputExpr(const InputExprPtr &expr) { return; }

  void resolveAssignmentExpr(const AssignmentExprPtr &expr) {
    if (!envManager.contains(expr->varName))
      envManager.define(expr->varName, false);
    resolveExpr(expr->initializer);
    envManager.assign(expr->varName, true);
  }

  void resolveUnaryExpr(const UnaryExprPtr &expr) {
    resolveExpr(expr->expression);
  }

  void resolveBinaryExpr(const BinaryExprPtr &expr) {
    resolveExpr(expr->lhsExpression);
    resolveExpr(expr->rhsExpression);
  }

  void resolvePostfixExpr(const PostfixExprPtr &expr) {
    resolveExpr(expr->expression);
  }

  void resolveVarStmt(const VarStmtPtr &stmt) {
    if (!envManager.contains(stmt->varName))
      envManager.define(stmt->varName, false);
    resolveExpr(stmt->initializer);
    envManager.assign(stmt->varName, true);
  }

  void resolveIfStmt(const IfStmtPtr &stmt) {
    resolveExpr(stmt->condition);
    resolveStmt(stmt->thenBranch);
    if (stmt->elseBranch)
      resolveStmt(*stmt->elseBranch);
  }

  void resolveBlockStmt(const BlockStmtPtr &stmt) {
    executeStmts(stmt->statements);
  }

  void resolveWhileStmt(const WhileStmtPtr &stmt) {
    resolveExpr(stmt->condition);
    resolveStmt(stmt->body);
  }

  void resolvePrintStmt(const PrintStmtPtr &stmt) { resolveExpr(stmt->value); }

  void resolveExprStmt(const ExprStmtPtr &stmt) {
    resolveExpr(stmt->expression);
  }

  ErrorReporter &eReporter;
  Evaluator::EnvironmentManager<bool> envManager;
};

} // namespace prsl::Resolver