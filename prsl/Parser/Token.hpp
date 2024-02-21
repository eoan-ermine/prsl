#pragma once

#include <functional>
#include <string>
#include <string_view>

namespace prsl::Types {

class Token {
public:
  enum class Type {
    COLON,
    COMMA,
    ELSE,
    EOF_,
    EQUAL_EQUAL,
    EQUAL,
    ERROR,
    FUNC,
    GREATER_EQUAL,
    GREATER,
    IDENT,
    IF,
    INPUT,
    LEFT_BRACE,
    LEFT_PAREN,
    LESS_EQUAL,
    LESS,
    MINUS_MINUS,
    MINUS,
    NOT_EQUAL,
    NUMBER,
    PLUS_PLUS,
    PLUS,
    PRINT,
    RETURN,
    RIGHT_BRACE,
    RIGHT_PAREN,
    SEMICOLON,
    SLASH,
    STAR,
    WHILE,
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
  constexpr Token(Type type, std::string_view str, int line) noexcept
      : type(type), lexeme(str), line(line) {}

  /**
   * Check if the token represents an error.
   *
   * @return true if the token represents an error, false otherwise
   *
   * @throws None
   */
  constexpr bool isError() const noexcept { return type == Type::ERROR; }

  /**
   * Get the type of the token.
   *
   * @return the type of the token
   *
   * @throws None
   */
  constexpr Type getType() const noexcept { return type; }

  /**
   * Get the lexeme of the token.
   *
   * @return the lexeme of the token
   *
   * @throws None
   */
  constexpr std::string_view getLexeme() const noexcept { return lexeme; }

  /**
   * Get the line number where the token appears.
   *
   * @return the line number
   *
   * @throws None
   */
  constexpr int getLine() const noexcept { return line; }

  /**
   * Converts the token to a string.
   *
   * @return the string representation of the token
   */
  constexpr std::string toString() const {
    if (type == Token::Type::EOF_)
      return "EOF";
    return {lexeme.data(), lexeme.size()};
  }

  constexpr auto operator==(const Token &rhs) const noexcept {
    return lexeme == rhs.lexeme;
  }

  constexpr auto operator<=>(const Token &rhs) const noexcept {
    return lexeme <=> rhs.lexeme;
  }

private:
  std::string_view lexeme;
  int line;
  Type type;
};

} // namespace prsl::Types

template <> struct std::hash<prsl::Types::Token> {
  std::size_t operator()(const prsl::Types::Token &token) const noexcept {
    return std::hash<std::string_view>()(token.getLexeme());
  }
};