#pragma once

#include <string>
#include <string_view>

namespace prsl::Types {

class Token {
public:
  enum class Type {
    IDENT,
    NUMBER,
    EQUAL,
    SEMICOLON,
    LEFT_PAREN,
    RIGHT_PAREN,
    PLUS_PLUS,
    MINUS_MINUS,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    GREATER,
    LESS,
    GREATER_EQUAL,
    LESS_EQUAL,
    EQUAL_EQUAL,
    NOT_EQUAL,
    IF,
    ELSE,
    LEFT_BRACE,
    RIGHT_BRACE,
    ERROR,
    EOF_
  };

  Token(Type type, std::string_view str, int line)
      : type(type), start(str.data()), length(str.size()), line(line) {}

  bool isError() const { return type == Type::ERROR; }

  Type getType() const { return type; }

  std::string_view getLexeme() const { return {start, length}; }

  int getLine() const { return line; }

  std::string toString() const {
    if (type == Token::Type::EOF_)
      return "<EOF>";
    return {start, length};
  }

private:
  Type type;
  const char *start;
  size_t length;
  int line;
};

} // namespace prsl::Types