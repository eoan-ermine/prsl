#pragma once

#include <string_view>

class Token {
public:
    enum class Type {
        IDENT,
        NUMBER,
        EQUALS,
        SEMICOLON,
        ERROR,
        EOF_
    };

    Token(Type type, std::string_view str, int line) : type(type), start(str.data()), length(str.size()), line(line) {}

    bool isError() {
        return type == Type::ERROR;
    }

    Type getType() const {
        return type;
    }

    std::string_view getLexeme() const {
        return {start, length};
    }

    int getLine() const {
        return line;
    }

private:
    Type type;
    const char *start;
    size_t length;
    int line;
};
