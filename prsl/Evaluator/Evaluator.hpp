#pragma once

#include <vector>

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

class Evaluator {
public:
  explicit Evaluator(ErrorReporter &eReporter);

  PrslObject evaluateExpr(const ExprPtrVariant &expr);
  void evaluateStmt(const StmtPtrVariant &stmt);

  void executeStmts(const std::vector<StmtPtrVariant> &stmts);
  void dump(std::string_view);

private:
  PrslObject evaluateLiteralExpr(const LiteralExprPtr &expr);
  PrslObject evaluateGroupingExpr(const GroupingExprPtr &expr);
  PrslObject evaluateVarExpr(const VarExprPtr &expr);
  PrslObject evaluateInputExpr(const InputExprPtr &expr);
  PrslObject evaluateAssignmentExpr(const AssignmentExprPtr &expr);
  PrslObject evaluateUnaryExpr(const UnaryExprPtr &expr);
  PrslObject evaluateBinaryExpr(const BinaryExprPtr &expr);
  PrslObject evaluatePostfixExpr(const PostfixExprPtr &expr);

  void evaluateVarStmt(const VarStmtPtr &stmt);
  void evaluateIfStmt(const IfStmtPtr &stmt);
  void evaluateBlockStmt(const BlockStmtPtr &stmt);
  void evaluateWhileStmt(const WhileStmtPtr &stmt);
  void evaluatePrintStmt(const PrintStmtPtr &stmt);
  void evaluateExprStmt(const ExprStmtPtr &stmt);

  int getInt(const Token &token, const PrslObject &obj);

private:
  ErrorReporter &eReporter;
  EnvironmentManager<PrslObject> envManager;
};

} // namespace prsl::Evaluator