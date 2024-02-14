#pragma once

#include "prsl/AST/NodeTypes.hpp"
#include "prsl/AST/TreeWalkerVisitor.hpp"
#include "prsl/Debug/ErrorReporter.hpp"
#include "prsl/Compiler/Common/Environment.hpp"
#include "prsl/Compiler/Common/FunctionsManager.hpp"

#include <filesystem>

namespace prsl::Semantics {

using namespace AST;

class Semantics : public TreeWalkerVisitor {
public:
  explicit Semantics(Errors::ErrorReporter &eReporter);
  bool dump(const std::filesystem::path &path) const;

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

private:
  Errors::ErrorReporter &eReporter;
  Types::EnvironmentManager<bool> envManager;
  Types::FunctionsManager<bool> functionsManager;
  bool inFunction = false;
};

} // namespace prsl::Semantics