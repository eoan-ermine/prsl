#pragma once

#include "Types/Token.hpp"
#include <cctype>
#include <string_view>
#include <vector>

class Scanner {
public:
    explicit Scanner(std::string_view source) : start(source.data()), current(source.data()) { }

    std::vector<Token> scanTokens() {
        std::vector<Token> tokens;
        while (!isAtEnd()) {
            tokens.push_back(scanToken());
        }
        tokens.push_back({Token::Type::EOF_, "", line});
        return tokens;
    }

    Token scanToken() {
        skipWhitespace();
        start = current;

        if (isAtEnd()) {
            return makeToken(Token::Type::EOF_);
        }

        char c = advance();
        if (std::isalpha(c)) {
            return ident();
        }
        if (c == '-' || (std::isdigit(c) && !(c == '0' && std::isdigit(peekNext())))) {
            return number();
        }

        switch (c) {
            case '=':
                return makeToken(Token::Type::EQUALS);
            case ';':
                return makeToken(Token::Type::SEMICOLON);
        }
    }
private:
    Token ident() {
        while (std::isalnum(peek()))
            advance();

        return makeToken(Token::Type::IDENT);
    }

    Token number() {
        auto error = makeError("Not a NUMBER");

        while (std::isdigit(peek()))
            advance();

        if (peek() == '.') {
            advance();
            if (!std::isdigit(peek()))
                return error;
            while (std::isdigit(peek()))
                advance();
        }

        return makeToken(Token::Type::NUMBER);
    }

    bool isAtEnd() {
        return *current == '\0';
    }

    char advance() {
        return *(current++);
    }

    char peek() {
        return *current;
    }

    char peekNext() {
        if (isAtEnd())
            return '\0';
        return current[-1];
    }

    void skipWhitespace() {
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

    Token makeToken(Token::Type type) {
        return {type, std::string_view{start, static_cast<size_t>(current - start)}, line};
    }

    Token makeError(std::string_view message) const {
        return {Token::Type::ERROR, message, line};
    }

private:
    const char *start;
    const char *current;
    int line;
};