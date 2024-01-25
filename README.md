# prsl

ParaCL language interpreter & transpiler (to LLVM IR)

## Getting the Source Code and Building prsl

1. Check out prsl:
  * `git clone git@github.com:eoan-ermine/prsl.git`
  * Or, on windows:
  `git clone --config core.autocrlf=false git@github.com:eoan-ermine/prsl.git`
2. Configure and build prsl:
  * `cd prsl`
  * `cmake -S . -B build -G <generator>`
    Some common build system generators are:
      * Ninja — for generating Ninja build files
      * Unix Makefiles — for generating make-compatible parallel makefiles
      * Visual Studio — for generating Visual Studio projects and solutions
      * Xcode — for generating Xcode projects
  * `cmake --build build [--target <target>]`
    * The default target (i.e `cmake --build build` or `make -C build`) will build all prsl
    * The `check-all` target (i.e `ninja check-all`) will run the regression tests to ensure everything is in working order
    * The `format` target (i.e `ninja format`) will run clang-format on all project files

## Usage

### Syntax & semantics checking mode

```shell
prsl --parse source.prsl
```

### Interpretation mode

```shell
prsl --interpret source.prsl
# Interpretation mode is the default
prsl source.prsl
```

### Compiling mode

```shell
prsl --codegen source.prsl --output-file source.ll
# Now we have LLVM IR, we can compile it with clang
clang source.ll -o source
# Now we can execute it!
./source
```

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
