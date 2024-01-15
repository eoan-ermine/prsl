#pragma once

#include "AST/NodeTypes.hpp"
#include <string_view>
#include <vector>
#include <charconv>

class Parser {
public:
    explicit Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) { }

    std::vector<StmtPtrVariant> parse() {
        std::vector<StmtPtrVariant> statements;
        while (!isAtEnd()) {
            statements.emplace_back(decl());
        }
        return statements;
    }


private:
    StmtPtrVariant decl() {
        if (match(Token::Type::IDENT))
            return varDecl();
    }

    StmtPtrVariant varDecl() {
        Token ident = previous();
        consume(Token::Type::EQUALS, "Expect equals sign after identifier.");
        ExprPtrVariant initializer = expr();
        consume(Token::Type::SEMICOLON, "Expect ';' after variable declaration.");
        return createVarSPV(ident, std::move(initializer));
    }

    ExprPtrVariant expr() {
        return primaryExpr();
    }

    ExprPtrVariant primaryExpr() {
        consume(Token::Type::NUMBER, "Expect number");
        
        auto view = previous().getView();
        int result{};
        auto [ptr, ec] = std::from_chars(view.begin(), view.end(), result);

        if (ec == std::errc())
            return createLiteralEPV(result);
    }

    bool match(Token::Type type) {
        if (check(type)) {
            advance();
            return true;
        }
        return false;
    }

    Token consume(Token::Type type, std::string_view message) {
        if (check(type))
            return advance();
    }

    bool check(Token::Type type) {
        if (isAtEnd())
            return false;
        return peek().getType() == type;
    }

    Token advance() {
        if (!isAtEnd())
            current++;
        return previous();
    }

    bool isAtEnd() {
        return peek().getType() == Token::Type::EOF_;
    }

    Token peek() {
        return tokens[current];
    }

    Token previous() {
        return tokens[current - 1];
    }

    std::vector<Token> tokens;
    int current = 0;
};