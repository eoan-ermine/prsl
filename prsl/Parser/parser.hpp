#pragma once

#include "prsl/AST/NodeTypes.hpp"
#include "prsl/Debug/ErrorReporter.hpp"
#include <charconv>
#include <initializer_list>
#include <optional>
#include <string_view>
#include <variant>
#include <vector>

namespace prsl::Parser {

using prsl::Types::Token;

using AST::ExprPtrVariant;
using AST::StmtPtrVariant;

class Parser {
public:
  explicit Parser(const std::vector<Token> &tokens,
                  Errors::ErrorReporter &eReporter)
      : tokens(tokens), eReporter(eReporter) {
    this->currentIter = this->tokens.begin();
  }

  class ParseError : public std::exception {}; // Exception types

  std::vector<StmtPtrVariant> parse() { return program(); }

private:
  std::vector<StmtPtrVariant> program() {
    std::vector<StmtPtrVariant> statements;
    try {
      while (!isEOF()) {
        statements.emplace_back(decl());
      }
    } catch (const ParseError &e) {
      synchronize();
    }
    return statements;
  }

  StmtPtrVariant decl() {
    if (match(Token::Type::IDENT) && matchNext(Token::Type::EQUAL)) {
      return varDecl();
    }

    return stmt();
  }

  StmtPtrVariant varDecl() {
    Token ident = getTokenAdvance();
    consumeOrError(Token::Type::EQUAL, "Expect equals sign after identifier");
    ExprPtrVariant initializer = expr();
    consumeOrError(Token::Type::SEMICOLON,
                   "Expect ';' after variable declaration");
    return createVarSPV(ident, std::move(initializer));
  }

  StmtPtrVariant stmt() {
    if (match(Token::Type::IF))
      return ifStmt();
    if (match(Token::Type::LEFT_BRACE))
      return blockStmt();
    if (match(Token::Type::WHILE))
      return whileStmt();
    if (match(Token::Type::PRINT))
      return printStmt();
    return exprStmt();
  }

  StmtPtrVariant ifStmt() {
    advance();
    consumeOrError(Token::Type::LEFT_PAREN, "Expect '(' after if");
    ExprPtrVariant condition = expr();
    consumeOrError(Token::Type::RIGHT_PAREN, "Expect ')' after if condition");

    StmtPtrVariant thenBranch = stmt();
    std::optional<StmtPtrVariant> elseBranch;
    if (match(Token::Type::ELSE)) {
      advance();
      elseBranch = stmt();
    }

    return AST::createIfSPV(std::move(condition), std::move(thenBranch),
                            std::move(elseBranch));
  }

  StmtPtrVariant blockStmt() {
    advance();
    std::vector<StmtPtrVariant> statements;
    while (!match(Token::Type::RIGHT_BRACE) && !isEOF()) {
      statements.push_back(decl());
    }
    consumeOrError(Token::Type::RIGHT_BRACE, "Expect '}' after block");
    return AST::createBlockSPV(std::move(statements));
  }

  StmtPtrVariant whileStmt() {
    advance();
    consumeOrError(Token::Type::LEFT_PAREN, "Expect '(' after while");
    ExprPtrVariant condition = expr();
    consumeOrError(Token::Type::RIGHT_PAREN, "Expect ')' after while condition");

    return AST::createWhileSPV(std::move(condition), stmt());
  }

  StmtPtrVariant printStmt() {
    advance();
    ExprPtrVariant value = expr();
    consumeOrError(Token::Type::SEMICOLON, "Expect ';' after print statement");
    return AST::createPrintSPV(std::move(value));
  }

  StmtPtrVariant exprStmt() {
    auto expression = expr();
    consumeOrError(Token::Type::SEMICOLON, "Expect ';' after expression statement");
    return AST::createExprSPV(std::move(expression));
  }

  ExprPtrVariant expr() { return assignmentExpr(); }

  ExprPtrVariant assignmentExpr() {
    auto expr = comparisonExpr();

    if (match(Token::Type::EQUAL)) {
      advance();
      if (std::holds_alternative<AST::VarExprPtr>(expr)) {
        Token varName = std::get<AST::VarExprPtr>(expr)->ident;
        return AST::createAssignmentEPV(varName, assignmentExpr());
      }

      throw error("Expect assignment target, got something else");
    }

    return expr;
  }

  ExprPtrVariant comparisonExpr() {
    auto comparatorTypes = {Token::Type::GREATER, Token::Type::GREATER_EQUAL,
                            Token::Type::LESS,    Token::Type::LESS_EQUAL,
                            Token::Type::EQUAL_EQUAL,   Token::Type::NOT_EQUAL, Token::Type::EQUAL_EQUAL};
    auto expr = additionExpr();
    while (match(comparatorTypes)) {
      Token op = getTokenAdvance();
      expr = createBinaryEPV(std::move(expr), op, additionExpr());
    }
    return expr;
  }

  ExprPtrVariant additionExpr() {
    auto additionTypes = {Token::Type::PLUS, Token::Type::MINUS};
    auto expr = multiplicationExpr();
    while (match(additionTypes)) {
      Token op = getTokenAdvance();
      expr = createBinaryEPV(std::move(expr), op, multiplicationExpr());
    }
    return expr;
  }

  ExprPtrVariant multiplicationExpr() {
    auto multiplicationTypes = {Token::Type::STAR, Token::Type::SLASH};
    auto expr = unaryExpr();
    while (match(multiplicationTypes)) {
      Token op = getTokenAdvance();
      expr = createBinaryEPV(std::move(expr), op, unaryExpr());
    }
    return expr;
  }

  ExprPtrVariant unaryExpr() {
    if (match(Token::Type::MINUS)) {
      auto op = getTokenAdvance();
      auto expr = postfixExpr();
      return createUnaryEPV(std::move(expr), op);
    }
    return postfixExpr();
  }

  ExprPtrVariant postfixExpr() {
    auto expr = primaryExpr();
    if (match({Token::Type::PLUS_PLUS, Token::Type::MINUS_MINUS})) {
      return createPostfixEPV(std::move(expr), getTokenAdvance());
    }
    return expr;
  }

  ExprPtrVariant primaryExpr() {
    if (match(Token::Type::NUMBER))
      return literalExpr();
    if (match(Token::Type::LEFT_PAREN))
      return groupingExpr();
    if (match(Token::Type::IDENT))
      return varExpr();
    if (match(Token::Type::INPUT))
      return inputExpr();

    throw error("Expect expression, got something else");
  }

  ExprPtrVariant literalExpr() {
    auto token = getTokenAdvance();
    auto view = token.getLexeme();
    int result{};
    auto [ptr, ec] = std::from_chars(view.begin(), view.end(), result);

    if (ec == std::errc())
      return AST::createLiteralEPV(result);

    throw error("Literal is not a number");
  }

  ExprPtrVariant groupingExpr() {
    advance();
    ExprPtrVariant expression = expr();
    consumeOrError(Token::Type::RIGHT_PAREN,
                   "Expect a closing paren after expression");
    return AST::createGroupingEPV(std::move(expression));
  }

  ExprPtrVariant varExpr() {
    Token varName = getTokenAdvance();
    return AST::createVarEPV(varName);
  }

  ExprPtrVariant inputExpr() {
    advance();
    return AST::createInputEPV();
  }

  void synchronize() {
    while (!isEOF()) {
      switch (getCurrentTokenType()) {
        case Token::Type::SEMICOLON: advance(); break;
        case Token::Type::WHILE:
        case Token::Type::IF:
        case Token::Type::PRINT:
          return;
        default:
          advance();
      }
    }
  }

  void advance() {
    if (!isEOF())
      ++currentIter;
  }
  Token getTokenAdvance() {
    Token token = peek();
    advance();
    return token;
  }
  Token consumeOrError(Token::Type tType, const std::string &errorMessage) {
    if (getCurrentTokenType() == tType)
      return getTokenAdvance();
    throw error(errorMessage + ", got: " + peek().toString());
  }
  auto error(const std::string &eMessage) -> ParseError {
    reportError(eMessage);
    return ParseError{};
  }
  [[nodiscard]] auto getCurrentTokenType() const -> Token::Type {
    return currentIter->getType();
  }
  [[nodiscard]] auto isEOF() const -> bool {
    return peek().getType() == Token::Type::EOF_;
  }
  [[nodiscard]] auto match(Token::Type type) -> bool {
    if (type == getCurrentTokenType()) {
      return true;
    }
    return false;
  }
  [[nodiscard]] auto match(std::initializer_list<Token::Type> types) -> bool {
    auto currentType = getCurrentTokenType();
    for (auto type : types) {
      if (type == currentType)
        return true;
    }
    return false;
  }
  [[nodiscard]] auto matchNext(Token::Type type) -> bool {
    advance();
    bool res = match(type);
    --currentIter;
    return res;
  }
  [[nodiscard]] auto peek() const -> Token { return *currentIter; };
  void reportError(const std::string &message) {
    const Token &token = peek();
    std::string error = message;
    if (token.getType() == Token::Type::EOF_)
      error = "at EOF: " + error;
    else
      error = "at '" + token.toString() + "': " + error;
    eReporter.setError(token.getLine(), error);
  }

  const std::vector<Token> &tokens;
  std::vector<Token>::const_iterator currentIter;
  prsl::Errors::ErrorReporter &eReporter;
};

} // namespace prsl::Parser