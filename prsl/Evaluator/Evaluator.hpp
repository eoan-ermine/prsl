#pragma once

#include "prsl/AST/ASTVisitor.hpp"
#include "prsl/AST/NodeTypes.hpp"
#include "prsl/Debug/ErrorReporter.hpp"
#include "prsl/Evaluator/Environment.hpp"
#include "prsl/Evaluator/Objects.hpp"
#include "prsl/Types/Token.hpp"

namespace prsl::Evaluator {

using namespace AST;

using Errors::ErrorReporter;

using Types::Token;
using Type = Types::Token::Type;

class Evaluator : public ASTVisitor<PrslObject> {
public:
  explicit Evaluator(ErrorReporter &eReporter);
  bool dump(std::string_view) override;

private:
  PrslObject visitLiteralExpr(const LiteralExprPtr &expr) override;
  PrslObject visitGroupingExpr(const GroupingExprPtr &expr) override;
  PrslObject visitVarExpr(const VarExprPtr &expr) override;
  PrslObject visitInputExpr(const InputExprPtr &expr) override;
  PrslObject visitAssignmentExpr(const AssignmentExprPtr &expr) override;
  PrslObject visitUnaryExpr(const UnaryExprPtr &expr) override;
  PrslObject visitBinaryExpr(const BinaryExprPtr &expr) override;
  PrslObject visitPostfixExpr(const PostfixExprPtr &expr) override;

  void visitVarStmt(const VarStmtPtr &stmt) override;
  void visitIfStmt(const IfStmtPtr &stmt) override;
  void visitBlockStmt(const BlockStmtPtr &stmt) override;
  void visitWhileStmt(const WhileStmtPtr &stmt) override;
  void visitPrintStmt(const PrintStmtPtr &stmt) override;
  void visitExprStmt(const ExprStmtPtr &stmt) override;
  void visitFunctionStmt(const FunctionStmtPtr &stmt) override;

  int getInt(const Token &token, const PrslObject &obj);

private:
  ErrorReporter &eReporter;
  EnvironmentManager<PrslObject> envManager;
};

} // namespace prsl::Evaluator