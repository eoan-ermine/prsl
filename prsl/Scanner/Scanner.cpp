#include "Scanner.hpp"

namespace prsl::Scanner {

Scanner::Scanner(std::string_view source)
    : start(source.data()), current(source.data()) {}

std::vector<Token> Scanner::tokenize() {
  std::vector<Token> tokens;
  while (!isEOL()) {
    tokens.push_back(tokenizeOne());
  }
  tokens.push_back({Token::Type::EOF_, "", line});
  return tokens;
}

Token Scanner::tokenizeOne() {
  skipWhitespace();
  start = current;

  if (isEOL()) {
    return makeToken(Token::Type::EOF_);
  }

  char c = advance();
  switch (c) {
  case '=':
    if (match('='))
      return makeToken(Token::Type::EQUAL_EQUAL);
    return makeToken(Token::Type::EQUAL);
  case ',':
    return makeToken(Token::Type::COMMA);
  case ':':
    return makeToken(Token::Type::COLON);
  case ';':
    return makeToken(Token::Type::SEMICOLON);
  case '(':
    return makeToken(Token::Type::LEFT_PAREN);
  case ')':
    return makeToken(Token::Type::RIGHT_PAREN);
  case '+':
    if (match('+'))
      return makeToken(Token::Type::PLUS_PLUS);
    return makeToken(Token::Type::PLUS);
  case '-':
    if (match('-'))
      return makeToken(Token::Type::MINUS_MINUS);
    return makeToken(Token::Type::MINUS);
  case '*':
    return makeToken(Token::Type::STAR);
  case '/': {
    if (match('/')) {
      while (peek() != '\n' && !isEOL())
        advance();
    } else if (match('*')) {
      while (!(peek() == '*' && peekNext() == '/') && !isEOL()) {
        if (peek() == '\n')
          line++;
        advance();
      }
      if (isEOL()) {
        return makeError("Multiline comment has no termination");
      }
      advance();
      advance();
    } else {
      return makeToken(Token::Type::SLASH);
    }
    return tokenizeOne();
  }
  case '>':
    if (match('='))
      return makeToken(Token::Type::GREATER_EQUAL);
    return makeToken(Token::Type::GREATER);
  case '<':
    if (match('='))
      return makeToken(Token::Type::LESS_EQUAL);
    return makeToken(Token::Type::LESS);
  case '!':
    if (match('='))
      return makeToken(Token::Type::NOT_EQUAL);
    return makeError("Expect '=' sign");
  case '{':
    return makeToken(Token::Type::LEFT_BRACE);
  case '}':
    return makeToken(Token::Type::RIGHT_BRACE);
  case '?':
    return makeToken(Token::Type::INPUT);
  default:
    if (std::isalpha(c)) {
      return ident();
    }
    if (c == '-' ||
        (std::isdigit(c) && !(c == '0' && std::isdigit(peekNext())))) {
      return number();
    }

    return makeError("Unknown character");
  }
}

Token Scanner::ident() {
  while (std::isalnum(peek()))
    advance();

  std::string_view view = {start, static_cast<size_t>(current - start)};
  auto it = keywords.find(view);
  if (it == keywords.end())
    return makeToken(Token::Type::IDENT);

  return makeToken(it->second);
}

Token Scanner::number() {
  while (std::isdigit(peek()))
    advance();

  return makeToken(Token::Type::NUMBER);
}

bool Scanner::isEOL() { return *current == '\0'; }

char Scanner::advance() { return *(current++); }

char Scanner::peek() { return *current; }

char Scanner::peekNext() {
  if (isEOL())
    return '\0';
  return current[1];
}

bool Scanner::match(char expect) {
  if (isEOL())
    return false;
  if (peek() != expect)
    return false;
  advance();
  return true;
}

void Scanner::skipWhitespace() {
  for (;;) {
    char c = peek();
    switch (c) {
    case ' ':
    case '\r':
    case '\t':
      advance();
      break;
    case '\n':
      line++;
      advance();
      break;
    default:
      return;
    }
  }
}

Token Scanner::makeToken(Token::Type type) {
  return {type, std::string_view{start, static_cast<size_t>(current - start)},
          line};
}

Token Scanner::makeError(std::string_view message) const {
  return {Token::Type::ERROR, message, line};
}

} // namespace prsl::Scanner