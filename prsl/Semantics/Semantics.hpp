#pragma once

#include "prsl/AST/NodeTypes.hpp"
#include "prsl/AST/TreeWalkerVisitor.hpp"
#include "prsl/Debug/ErrorReporter.hpp"
#include "prsl/Evaluator/Environment.hpp"
#include "prsl/Types/Token.hpp"
#include <string_view>
#include <unordered_map>

namespace prsl::Semantics {

using namespace AST;

using Errors::ErrorReporter;

using Types::Token;
using Type = Types::Token::Type;

class Semantics : public TreeWalkerVisitor {
public:
  explicit Semantics(ErrorReporter &eReporter);
  bool dump(std::string_view filename) override;

private:
  void visitVarExpr(const VarExprPtr &expr) override;
  void visitAssignmentExpr(const AssignmentExprPtr &expr) override;
  void visitPostfixExpr(const PostfixExprPtr &expr) override;
  void visitScopeExpr(const ScopeExprPtr &expr) override;
  void visitFuncExpr(const FuncExprPtr &expr) override;
  void visitCallExpr(const CallExprPtr &expr) override;

  void visitVarStmt(const VarStmtPtr &stmt) override;
  void visitBlockStmt(const BlockStmtPtr &stmt) override;
  void visitReturnStmt(const ReturnStmtPtr &stmt) override;

  ErrorReporter &eReporter;
  struct VarState {
    bool isInit;
    enum Type { VAR, FUNCTION } type;
  };
  Evaluator::EnvironmentManager<VarState> envManager;
  std::unordered_map<std::string_view, bool> functionsManager;
  bool inFunction = false;
};

} // namespace prsl::Semantics