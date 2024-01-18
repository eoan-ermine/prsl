#pragma once

#include <memory>
#include <variant>
#include "../Types/Token.hpp"

namespace prsl::AST {

struct LiteralExpr;

using LiteralExprPtr = std::unique_ptr<LiteralExpr>;

using ExprPtrVariant = std::variant<LiteralExprPtr>;

struct VarStmt;

using VarStmtPtr = std::unique_ptr<VarStmt>;

using StmtPtrVariant = std::variant<VarStmtPtr>;

struct LiteralExpr final {
    int literalVal;
    explicit LiteralExpr(int value) {
        literalVal = value;
    }
};

using prsl::Types::Token;

struct VarStmt final {
    Token varName;
    ExprPtrVariant initializer;
    explicit VarStmt(Token varName, ExprPtrVariant initializer) : varName(varName), initializer(std::move(initializer)) { }
};

inline auto createLiteralEPV(int literalVal) -> ExprPtrVariant {
    return std::make_unique<LiteralExpr>(literalVal);
}

inline auto createVarSPV(Token varName, ExprPtrVariant initializer) -> StmtPtrVariant {
    return std::make_unique<VarStmt>(varName, std::move(initializer));
}

}