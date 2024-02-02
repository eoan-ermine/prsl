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
    WHILE,
    INPUT,
    PRINT,
    FUNC,
    COMMA,
    COLON,
    ERROR,
    EOF_
  };

  /**
   * Constructor for Token class.
   *
   * @param type the type of the token
   * @param str the string representation of the token
   * @param line the line number where the token appears
   *
   * @return None
   *
   * @throws None
   */
  Token(Type type, std::string_view str, int line) noexcept
      : type(type), lexeme(str), line(line) {}

  /**
   * Check if the token represents an error.
   *
   * @return true if the token represents an error, false otherwise
   *
   * @throws None
   */
  bool isError() const noexcept { return type == Type::ERROR; }

  /**
   * Get the type of the token.
   *
   * @return the type of the token
   *
   * @throws None
   */
  Type getType() const noexcept { return type; }

  /**
   * Get the lexeme of the token.
   *
   * @return the lexeme of the token
   *
   * @throws None
   */
  std::string_view getLexeme() const noexcept { return lexeme; }

  /**
   * Get the line number where the token appears.
   *
   * @return the line number
   *
   * @throws None
   */
  int getLine() const noexcept { return line; }

  /**
   * Converts the token to a string.
   *
   * @return the string representation of the token
   */
  std::string toString() const {
    if (type == Token::Type::EOF_)
      return "EOF";
    return {lexeme.data(), lexeme.size()};
  }

private:
  Type type;
  std::string_view lexeme;
  int line;
};

} // namespace prsl::Types