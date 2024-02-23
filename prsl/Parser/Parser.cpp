#include "prsl/Parser/Parser.hpp"
#include "prsl/AST/NodeTypes.hpp"
#include "prsl/Debug/Errors.hpp"

#include <charconv>

namespace prsl::Parser {

using prsl::Types::Token;

using AST::ExprPtrVariant;
using AST::StmtPtrVariant;

Parser::Parser(const std::vector<Token> &tokens, Errors::Logger &logger)
    : tokens(tokens), logger(logger) {
  this->currentIter = this->tokens.begin();
}

StmtPtrVariant Parser::parse() { return program(); }

//  <program> ::=
//    <decl>*
StmtPtrVariant Parser::program() {
  std::vector<StmtPtrVariant> statements;
  try {
    while (!isEOF()) {
      statements.emplace_back(decl());
    }
  } catch (const Errors::ParseError &e) {
    synchronize();
  }
  return createFunctionSPV({}, std::move(statements));
}

// <decl> ::=
//   <stmt>
//   | <varDecl>
StmtPtrVariant Parser::decl() {
  if (match(Token::Type::IDENT) && matchNext(Token::Type::EQUAL)) {
    return varDecl();
  }

  return stmt();
}

// <varDecl> ::=
//   <ident> "=" <expr> ";"?
StmtPtrVariant Parser::varDecl() {
  Token ident = getTokenAdvance();
  consumeOrError(Token::Type::EQUAL, "Expect equals sign after identifier");
  ExprPtrVariant initializer = expr();
  if (match(Token::Type::SEMICOLON))
    advance();
  return createVarSPV(ident, std::move(initializer));
}

// <stmt> ::=
//   <ifStmt>
//   | <whileStmt>
//   | <printStmt>
//   | <exprStmt>
//   | <blockStmt>
//   | <returnStmt>
//   | <nullStmt>
StmtPtrVariant Parser::stmt() {
  if (match(Token::Type::IF))
    return ifStmt();
  if (match(Token::Type::LEFT_BRACE))
    return blockStmt();
  if (match(Token::Type::WHILE))
    return whileStmt();
  if (match(Token::Type::PRINT))
    return printStmt();
  if (match(Token::Type::RETURN))
    return returnStmt();
  if (match(Token::Type::SEMICOLON))
    return nullStmt();
  return exprStmt();
}

// <ifStmt> ::=
//   "if(" <expr> ")" <stmt>
//   | "if(" <expr> ")" <stmt> "else" <stmt>
StmtPtrVariant Parser::ifStmt() {
  advance();
  consumeOrError(Token::Type::LEFT_PAREN, "Expect '(' after if");
  ExprPtrVariant condition = expr();
  consumeOrError(Token::Type::RIGHT_PAREN, "Expect ')' after if condition");

  StmtPtrVariant thenBranch = stmt();
  std::optional<StmtPtrVariant> elseBranch;
  if (match(Token::Type::ELSE)) {
    advance();
    elseBranch = stmt();
  }

  return AST::createIfSPV(std::move(condition), std::move(thenBranch),
                          std::move(elseBranch));
}

// <blockStmt> ::=
//   "{" <program> "}"
StmtPtrVariant Parser::blockStmt() {
  advance();
  std::vector<StmtPtrVariant> statements;
  while (!match(Token::Type::RIGHT_BRACE) && !isEOF()) {
    statements.emplace_back(decl());
  }
  consumeOrError(Token::Type::RIGHT_BRACE, "Expect '}' after block");
  return AST::createBlockSPV(std::move(statements));
}

// <whileStmt> ::=
//   "while(" <expr> ")" <stmt>
StmtPtrVariant Parser::whileStmt() {
  advance();
  consumeOrError(Token::Type::LEFT_PAREN, "Expect '(' after while");
  ExprPtrVariant condition = expr();
  consumeOrError(Token::Type::RIGHT_PAREN, "Expect ')' after while condition");

  return AST::createWhileSPV(std::move(condition), stmt());
}

// <printStmt> ::=
//   "print" <expr> ";"
StmtPtrVariant Parser::printStmt() {
  advance();
  ExprPtrVariant value = expr();
  consumeOrError(Token::Type::SEMICOLON, "Expect ';' after print statement");
  return AST::createPrintSPV(std::move(value));
}

// <returnStmt> ::=
//   "return" <expr> ";"
StmtPtrVariant Parser::returnStmt() {
  auto token = getTokenAdvance();
  ExprPtrVariant value = expr();
  consumeOrError(Token::Type::SEMICOLON, "Expect ';' after return statement");
  return AST::createReturnSPV(token, std::move(value), true);
}

// <exprStmt> ::=
//   <expr> ";"
StmtPtrVariant Parser::exprStmt() {
  auto expression = expr();
  consumeOrError(Token::Type::SEMICOLON,
                 "Expect ';' after expression statement");
  return AST::createExprSPV(std::move(expression));
}

// <nullStmt> ::=
//   ";"
StmtPtrVariant Parser::nullStmt() {
  advance();
  return AST::createNullSPV();
}

// <expr> ::=
//   <assignmentExpr>
ExprPtrVariant Parser::expr() { return assignmentExpr(); }

// <assignmentExpr> ::=
//   <comparisonExpr>
//   | <comparisonExpr> "=" <assignmentExpr>
ExprPtrVariant Parser::assignmentExpr() {
  auto expr = comparisonExpr();

  if (match(Token::Type::EQUAL)) {
    advance();
    if (std::holds_alternative<AST::VarExprPtr>(expr)) {
      Token varName = std::get<AST::VarExprPtr>(expr)->ident;
      return AST::createAssignmentEPV(varName, assignmentExpr());
    }

    throw error("Expect assignment target, got something else");
  }

  return expr;
}

// <comparisonExpr> ::=
//   <additionExpr>
//   | <additionExpr> ">" <additionExpr>
//   | <additionExpr> ">=" <additionExpr>
//   | <additionExpr> "<" <additionExpr>
//   | <additionExpr> "<=" <additionExpr>
//   | <additionExpr> "!=" <additionExpr>
//   | <additionExpr> "==" <additionExpr>
ExprPtrVariant Parser::comparisonExpr() {
  auto comparatorTypes = {Token::Type::GREATER,     Token::Type::GREATER_EQUAL,
                          Token::Type::LESS,        Token::Type::LESS_EQUAL,
                          Token::Type::EQUAL_EQUAL, Token::Type::NOT_EQUAL,
                          Token::Type::EQUAL_EQUAL};
  auto expr = additionExpr();
  while (match(comparatorTypes)) {
    Token op = getTokenAdvance();
    expr = createBinaryEPV(std::move(expr), op, additionExpr());
  }
  return expr;
}

// <additionExpr> ::=
//   <multiplicationExpr>
//   | <multiplicationExpr> "+" <multiplicationExpr>
//   | <multiplicationExpr> "-" <multiplicationExpr>
ExprPtrVariant Parser::additionExpr() {
  auto additionTypes = {Token::Type::PLUS, Token::Type::MINUS};
  auto expr = multiplicationExpr();
  while (match(additionTypes)) {
    Token op = getTokenAdvance();
    expr = createBinaryEPV(std::move(expr), op, multiplicationExpr());
  }
  return expr;
}

// <multiplicationExpr> ::=
//   <unaryExpr>
//   | <unaryExpr> "*" <unaryExpr>
//   | <unaryExpr> "/" <unaryExpr>
ExprPtrVariant Parser::multiplicationExpr() {
  auto multiplicationTypes = {Token::Type::STAR, Token::Type::SLASH};
  auto expr = unaryExpr();
  while (match(multiplicationTypes)) {
    Token op = getTokenAdvance();
    expr = createBinaryEPV(std::move(expr), op, unaryExpr());
  }
  return expr;
}

// <unaryExpr> ::=
//   <postfixExpr>
//   | "-" <postfixExpr>
ExprPtrVariant Parser::unaryExpr() {
  if (match(Token::Type::MINUS)) {
    auto op = getTokenAdvance();
    auto expr = postfixExpr();
    return createUnaryEPV(std::move(expr), op);
  }
  return postfixExpr();
}

// <postfixExpr> ::=
//   <callExpr>
//   | <callExpr> "++"
//   | <callExpr> "--"
ExprPtrVariant Parser::postfixExpr() {
  auto expr = callExpr();
  if (match({Token::Type::PLUS_PLUS, Token::Type::MINUS_MINUS})) {
    return createPostfixEPV(std::move(expr), getTokenAdvance());
  }
  return expr;
}

// <callExpr> ::=
//   <primaryExpr> "(" <arguments>? ")"
ExprPtrVariant Parser::callExpr() {
  auto expr = primaryExpr();

  if (match(Token::Type::LEFT_PAREN)) {
    if (!std::holds_alternative<AST::VarExprPtr>(expr)) {
      throw error("Expect variable expression, got something else");
    }
    auto ident = std::get<AST::VarExprPtr>(expr)->ident;
    advance();
    std::vector<ExprPtrVariant> args;
    if (!match(Token::Type::RIGHT_PAREN))
      args = arguments();
    consumeOrError(Token::Type::RIGHT_PAREN, "Expect ')' after arguments");
    expr = createCallEPV(std::move(ident), std::move(args));
  }

  return expr;
}

// <arguments> ::=
//   <assignmentExpr> ("," <assignmentExpr>)*
std::vector<ExprPtrVariant> Parser::arguments() {
  std::vector<ExprPtrVariant> args;
  args.emplace_back(assignmentExpr());
  while (match(Token::Type::COMMA)) {
    advance();
    args.emplace_back(assignmentExpr());
  }
  return args;
}

// <primaryExpr> ::=
//   <literalExpr>
//   | <groupingExpr>
//   | <varExpr>
//   | <inputExpr>
//   | <funcExpr>
//   | <scopeExpr>
ExprPtrVariant Parser::primaryExpr() {
  if (match(Token::Type::NUMBER))
    return literalExpr();
  if (match(Token::Type::LEFT_PAREN))
    return groupingExpr();
  if (match(Token::Type::IDENT))
    return varExpr();
  if (match(Token::Type::INPUT))
    return inputExpr();
  if (match(Token::Type::FUNC))
    return funcExpr();
  if (match(Token::Type::LEFT_BRACE))
    return scopeExpr();

  throw error("Expect expression, got something else");
}

// <literalExpr> ::=
//   <number>
ExprPtrVariant Parser::literalExpr() {
  auto token = getTokenAdvance();
  auto view = token.getLexeme();
  int result{};
  auto [ptr, ec] = std::from_chars(view.begin(), view.end(), result);

  if (ec == std::errc())
    return AST::createLiteralEPV(result);

  throw error("Literal is not a number");
}

// <groupingExpr> ::=
//   "(" <expr> ")"
ExprPtrVariant Parser::groupingExpr() {
  advance();
  ExprPtrVariant expression = expr();
  consumeOrError(Token::Type::RIGHT_PAREN,
                 "Expect a closing paren after expression");
  return AST::createGroupingEPV(std::move(expression));
}

// <varExpr> ::=
//   <ident>
ExprPtrVariant Parser::varExpr() {
  Token varName = getTokenAdvance();
  return AST::createVarEPV(varName);
}

// <inputExpr> ::=
//   "?"
ExprPtrVariant Parser::inputExpr() {
  advance();
  return AST::createInputEPV();
}

// <scopeExpr> ::=
//   "{" <program> "}"
ExprPtrVariant Parser::scopeExpr() {
  auto beginBrace = peek(); // For diagnostics purposes
  advance();
  std::vector<StmtPtrVariant> statements;
  bool hasReturn{false};
  while (!match(Token::Type::RIGHT_BRACE) && !isEOF()) {
    auto declaration = decl();
    if (!hasReturn && std::holds_alternative<AST::ReturnStmtPtr>(declaration)) {
      hasReturn = true;
    }
    statements.push_back(std::move(declaration));
  }

  auto unknownPos = Utils::FilePos::UNKNOWN();
  auto returnToken =
      Token{Token::Type::RETURN, "return", unknownPos, unknownPos};

  if (statements.size() &&
      std::holds_alternative<AST::ExprStmtPtr>(statements.back())) {
    auto expr =
        std::move(std::get<AST::ExprStmtPtr>(statements.back())->expression);

    if (std::holds_alternative<AST::FuncExprPtr>(expr)) {
      const auto &func = std::get<AST::FuncExprPtr>(expr);
      throw Errors::reportParseError(
          logger, func->token, "Can not return a function from the function");
    }

    statements.back() =
        AST::createReturnSPV(returnToken, std::move(expr), isFunction);
  } else if (!hasReturn) {
    logger.warning(beginBrace.getStartPos(), "Scope expression implicitly returns 0");
    statements.emplace_back(AST::createReturnSPV(
        returnToken, AST::createLiteralEPV(0), isFunction));
  }

  consumeOrError(Token::Type::RIGHT_BRACE, "Expect '}' after scope");
  return AST::createScopeEPV(std::move(statements));
}

// <funcExpr> ::=
//   "func(" (<ident> ("," <ident>)*)? ")" (":" <ident>)? <scopeExpr>
ExprPtrVariant Parser::funcExpr() {
  auto token = getTokenAdvance();
  consumeOrError(Token::Type::LEFT_PAREN, "Expect '(' after 'func'");
  std::vector<Token> parameters;

  if (match(Token::Type::IDENT)) {
    parameters.emplace_back(getTokenAdvance());
    while (match(Token::Type::COMMA)) {
      advance();
      parameters.emplace_back(getTokenAdvance());
    }
  }
  consumeOrError(Token::Type::RIGHT_PAREN, "Expect ')' after arguments");

  std::optional<Token> name = std::nullopt;
  if (match(Token::Type::COLON)) {
    advance();
    name = consumeOrError(Token::Type::IDENT, "Expect function name");
  }

  if (!match(Token::Type::LEFT_BRACE))
    throw error("Expect '{' before function body");

  bool previousIsFunction = isFunction;
  isFunction = true;
  auto func = std::get<std::unique_ptr<AST::FuncExpr>>(AST::createFuncEPV(
      std::move(token), std::move(name), std::move(parameters)));
  func->body = scopeExpr();
  isFunction = previousIsFunction;
  return std::move(func);
}

void Parser::synchronize() {
  while (!isEOF()) {
    switch (getCurrentTokenType()) {
    case Token::Type::SEMICOLON:
      advance();
      break;
    case Token::Type::WHILE:
    case Token::Type::IF:
    case Token::Type::PRINT:
      return;
    default:
      advance();
    }
  }
}

void Parser::advance() noexcept {
  if (!isEOF())
    ++currentIter;
}

Token Parser::getTokenAdvance() noexcept {
  Token token = peek();
  advance();
  return token;
}

Token Parser::consumeOrError(Token::Type tType, std::string_view errorMessage) {
  if (getCurrentTokenType() == tType)
    return getTokenAdvance();
  throw error(std::string(errorMessage) + ", got: " + peek().toString());
}

[[nodiscard]] Token::Type Parser::getCurrentTokenType() const noexcept {
  return currentIter->getType();
}

[[nodiscard]] bool Parser::isEOF() const noexcept {
  return peek().getType() == Token::Type::EOF_;
}

[[nodiscard]] bool Parser::match(Token::Type type) const noexcept {
  if (type == getCurrentTokenType()) {
    return true;
  }
  return false;
}

[[nodiscard]] bool
Parser::match(std::initializer_list<Token::Type> types) const noexcept {
  auto currentType = getCurrentTokenType();
  return std::any_of(types.begin(), types.end(),
                     [currentType](const auto &x) { return x == currentType; });
}

[[nodiscard]] bool Parser::matchNext(Token::Type type) noexcept {
  advance();
  bool res = match(type);
  --currentIter;
  return res;
}

[[nodiscard]] Token Parser::peek() const noexcept { return *currentIter; };

Errors::ParseError Parser::error(const std::string &msg) {
  return Errors::reportParseError(logger, peek(), msg);
}

} // namespace prsl::Parser