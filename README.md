# prsl

ParaCL language interpreter

## Language description

### EBNF

```ebnf
<letter> ::=
  [a-z]
  | [A-Z]

<ident> ::=
  <letter>
  | <ident> <letter>
  | <ident> [0-9]

<integer> ::=
  "0"
  | [1-9] [0-9]?

<number> ::=
  <integer>
  | <integer> "." [0-9]+

<program> ::=
  <decl>*

<decl> ::=
  <stmt>
  | <varDecl>

<varDecl> ::=
  <ident> "=" <expr> ";"

<stmt> ::=
  <ifStmt>
  | <blockStmt>
  | <whileStmt>
  | <printStmt>

<ifStmt> ::=
  "if(" <expr> "){" <stmt> "}"
  | "if(" <expr> "){" <stmt> "}else{" <stmt> "}"

<blockStmt> ::=
  "{" <program> "}"
 
<whileStmt> ::=
  "while(" <expr> "){" <stmt> "}"

<printStmt> ::=
  "print" <expr> ";"

<expr> ::=
  <assignmentExpr>

<assignmentExpr> ::=
  <comparisonExpr>
  | <comparisonExpr> "=" <assignmentExpr>

<comparisonExpr> ::=
  <additionExpr>
  | <additionExpr> ">" <additionExpr>
  | <additionExpr> ">=" <additionExpr>
  | <additionExpr> "<" <additionExpr>
  | <additionExpr> "<=" <additionExpr>
  | <additionExpr> "!=" <additionExpr>
  | <additionExpr> "==" <additionExpr>

<additionExpr> ::=
  <multiplicationExpr>
  | <multiplicationExpr> "+" <multiplicationExpr>
  | <multiplicationExpr> "-" <multiplicationExpr>

<multiplicationExpr> ::=
  <unaryExpr>
  | <unaryExpr> "*" <unaryExpr>
  | <unaryExpr> "/" <unaryExpr>

<unaryExpr> ::=
  <postfixExpr>
  | "-" <postfixExpr>
    
<postfixExpr> ::=
  <primaryExpr>
  | <primaryExpr> "++"
  | <primaryExpr> "--"

<primaryExpr> ::=
  <literalExpr>
  | <groupingExpr>
  | <varExpr>
  | <inputExpr>

<literalExpr> ::=
  <number>

<groupingExpr> ::=
  "(" <expr> ")"

<varExpr> ::=
  <ident>
    
<inputExpr> ::=
  "?"
```
