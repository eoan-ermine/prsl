#pragma once

#include "prsl/AST/NodeTypes.hpp"
#include "prsl/Debug/ErrorReporter.hpp"
#include <charconv>
#include <string_view>
#include <vector>

namespace prsl::Parser {

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

  std::vector<StmtPtrVariant> parse() {
    return program();
  }

private:
  std::vector<StmtPtrVariant> program() {
    std::vector<StmtPtrVariant> statements;
    try {
      while (!isEOF()) {
        statements.emplace_back(decl());
      }
    } catch (const ParseError &e) { }
    return statements;
  }

  StmtPtrVariant decl() {
    if (match(Token::Type::IDENT))
      return varDecl();
    throw error("Expect variable declaration, got something else");
  }

  StmtPtrVariant varDecl() {
    Token ident = getTokenAdvance();
    consumeOrError(Token::Type::EQUALS, "Expect equals sign after identifier.");
    ExprPtrVariant initializer = expr();
    consumeOrError(Token::Type::SEMICOLON,
                   "Expect ';' after variable declaration.");
    return createVarSPV(ident, std::move(initializer));
  }

  ExprPtrVariant expr() { return primaryExpr(); }

  ExprPtrVariant primaryExpr() {
    auto token = consumeOrError(Token::Type::NUMBER, "Expect number");

    auto view = token.getLexeme();
    int result{};
    auto [ptr, ec] = std::from_chars(view.begin(), view.end(), result);

    if (ec == std::errc())
      return AST::createLiteralEPV(result);

    throw error("Literal is not a number");
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
    throw error(errorMessage + " Got: " + peek().toString());
  }
  auto error(const std::string &eMessage) -> ParseError {
    reportError(eMessage);
    return ParseError{};
  }
  [[nodiscard]] auto getCurrentTokenType() const -> Token::Type {
    return currentIter->getType();
  }
  auto getTokenAndAdvance() -> Token {
    Token token = peek();
    advance();
    return token;
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
  [[nodiscard]] auto peek() const -> Token { return *currentIter; };
  void reportError(const std::string &message) {
    const Token &token = peek();
    std::string error = message;
    if (token.getType() == Token::Type::EOF_)
      error = " at EOF: " + error;
    else
      error = " at '" + token.toString() + "': " + error;
    eReporter.setError(token.getLine(), error);
  }

  const std::vector<Token> &tokens;
  std::vector<Token>::const_iterator currentIter;
  prsl::Errors::ErrorReporter &eReporter;
};

} // namespace prsl::Parser