#pragma once

#include "prsl/Parser/Token.hpp"

#include <string_view>
#include <unordered_map>
#include <vector>

namespace prsl::Scanner {

using prsl::Types::Token;

class Scanner {
public:
  explicit Scanner(std::string_view source);
  std::vector<Token> tokenize();
  Token tokenizeOne();

private:
  Token ident();
  Token number();

  bool isEOL();
  char advance();
  char peek();
  char peekNext();
  bool match(char expect);
  void skipWhitespace();

  Token makeToken(Token::Type type);
  Token makeError(std::string_view message) const;

private:
  const char *start;
  const char *current;
  int line{1};

  std::unordered_map<std::string_view, Token::Type> keywords = {
      {"if", Token::Type::IF},       {"else", Token::Type::ELSE},
      {"while", Token::Type::WHILE}, {"print", Token::Type::PRINT},
      {"func", Token::Type::FUNC},   {"return", Token::Type::RETURN}};
};

} // namespace prsl::Scanner