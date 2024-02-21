#pragma once

#include "prsl/AST/NodeTypes.hpp"
#include "prsl/Debug/Errors.hpp"
#include "prsl/Debug/Logger.hpp"

#include <initializer_list>
#include <string_view>
#include <vector>

namespace prsl::Parser {

using prsl::Types::Token;

using AST::ExprPtrVariant;
using AST::StmtPtrVariant;

class Parser {
public:
  explicit Parser(const std::vector<Token> &tokens,
                  Errors::Logger &logger);

  StmtPtrVariant parse();

private:
  StmtPtrVariant program();

  StmtPtrVariant decl();
  StmtPtrVariant varDecl();
  StmtPtrVariant stmt();
  StmtPtrVariant ifStmt();
  StmtPtrVariant whileStmt();
  StmtPtrVariant printStmt();
  StmtPtrVariant exprStmt();
  StmtPtrVariant blockStmt();
  StmtPtrVariant returnStmt();
  StmtPtrVariant nullStmt();

  ExprPtrVariant expr();
  ExprPtrVariant assignmentExpr();
  ExprPtrVariant comparisonExpr();
  ExprPtrVariant additionExpr();
  ExprPtrVariant multiplicationExpr();
  ExprPtrVariant unaryExpr();
  ExprPtrVariant postfixExpr();
  ExprPtrVariant primaryExpr();
  ExprPtrVariant literalExpr();
  ExprPtrVariant groupingExpr();
  ExprPtrVariant varExpr();
  ExprPtrVariant inputExpr();
  ExprPtrVariant scopeExpr();
  ExprPtrVariant funcExpr();
  ExprPtrVariant callExpr();
  std::vector<ExprPtrVariant> arguments();

  void synchronize();
  void advance() noexcept;
  Token getTokenAdvance() noexcept;
  Token consumeOrError(Token::Type tType, std::string_view errorMessage);
  Errors::ParseError error(const std::string &msg) {
    return Errors::reportParseError(logger, peek(), msg);
  }
  [[nodiscard]] Token::Type getCurrentTokenType() const noexcept;
  [[nodiscard]] bool isEOF() const noexcept;
  [[nodiscard]] bool match(Token::Type type) const noexcept;
  [[nodiscard]] bool
  match(std::initializer_list<Token::Type> types) const noexcept;
  [[nodiscard]] bool matchNext(Token::Type type) noexcept;
  [[nodiscard]] Token peek() const noexcept;

  const std::vector<Token> &tokens;
  std::vector<Token>::const_iterator currentIter;
  prsl::Errors::Logger &logger;
  bool isFunction{false};
};

} // namespace prsl::Parser