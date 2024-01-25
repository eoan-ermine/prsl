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
  <declaration>
  | <program> <declaration>

<declaration> ::=
  <statement>
  | <varDeclaration>

<varDeclaration> ::=
  <ident> "=" <expr> ";"

<statement> ::=
  <ifStatement>
  | <blockStatement>
  | <whileStatement>
  | <printStatement>

<ifStatement> ::=
  "if(" <expr> "){" <statement> "}"
  | "if(" <expr> "){" <statement> "}else{" <statement> "}"

<blockStatement> ::=
  "{" <program> "}"
 
<whileStatement> ::=
  "while(" <expr> "){" <statement> "}"

<printStatement> ::=
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