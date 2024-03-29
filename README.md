# prsl

![](https://github.com/eoan-ermine/prsl/actions/workflows/style.yaml/badge.svg) ![](https://github.com/eoan-ermine/prsl/actions/workflows/cppcheck.yaml/badge.svg) ![](https://github.com/eoan-ermine/prsl/actions/workflows/analyze.yaml/badge.svg) ![](https://github.com/eoan-ermine/prsl/actions/workflows/linux-gcc.yaml/badge.svg) ![](https://github.com/eoan-ermine/prsl/actions/workflows/linux-clang.yaml/badge.svg) ![](https://github.com/eoan-ermine/prsl/actions/workflows/documentation.yaml/badge.svg)

ParaCL language interpreter & transpiler (to LLVM IR)

## Dependencies

| Package              | Version |
| -------------------- | ------- |
| Boost.ProgramOptions | >=1.74  |
| LLVM                 | >=18    |

**Development dependencies**

| Package      | Version  |
| ------------ | -------- |
| lit          | >=17     |
| filecheck    | >=0.0.24 |
| clang-format | >= 17    |

**SAST Tools**

1. [PVS-Studio](https://pvs-studio.ru/ru/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source) - static analyzer for C, C++, C#, and Java code.
2. [Cppcheck](https://cppcheck.sourceforge.io/) — statis analysis tool for C/C++ code.

### Installation

#### Ubuntu

```shell
# Install Boost.Program Options
sudo apt install libboost-program-options-dev
# Install LLVM
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 18
```

**Development dependencies**:

```shell
pip install lit
pip install filecheck
sudo apt install clang-format
sudo apt install cppcheck
# For installation of PVS-Studio see PVS-Studio docs: https://pvs-studio.com/en/docs/manual/0039/#ID3ABD56C167
```

#### Arch

```shell
sudo pacman -S llvm
sudo pacman -S boost
```

**Development dependencies**:

```shell
pip install lit
pip install filecheck
sudo pacman -S clang # For clang-format
sudo pacman -S cppcheck
# For installation of PVS-Studio see PVS-Studio docs: https://pvs-studio.com/en/docs/manual/0039/#ID3ABD56C167

```

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
    * The `check-format` target (i.e `ninja check-format`) will verify that project's code follows formatting conventions
    * The `docs` target (i.e `ninja docs`) will generate documentation using doxygen
    * The `cppcheck` target (i.e `ninja cppcheck`) will run cppcheck on all project files
    * The `pvs-studio` target (i.e `ninja pvs-studio`) will run PVS-Studio on all project files

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
  <ident> "=" <expr> ";"?

<stmt> ::=
  <ifStmt>
  | <whileStmt>
  | <printStmt>
  | <exprStmt>
  | <blockStmt>
  | <returnStmt>
  | <nullStmt>

<ifStmt> ::=
  "if(" <expr> ")" <stmt>
  | "if(" <expr> ")" <stmt> "else" <stmt>
 
<whileStmt> ::=
  "while(" <expr> ")" <stmt>

<printStmt> ::=
  "print" <expr> ";"

<exprStmt> ::=
  <expr> ";"

<blockStmt> ::=
  "{" <program> "}"

<returnStmt> ::=
  "return" <expr> ";"

<nullStmt> ::=
  ";"

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
  <callExpr>
  | <callExpr> "++"
  | <callExpr> "--"

<callExpr> ::=
  <primaryExpr> "(" <arguments>? ")"

<arguments> ::=
  <assignmentExpr> ("," <assignmentExpr>)*

<primaryExpr> ::=
  <literalExpr>
  | <groupingExpr>
  | <varExpr>
  | <inputExpr>
  | <funcExpr>
  | <scopeExpr>

<literalExpr> ::=
  <number>

<groupingExpr> ::=
  "(" <expr> ")"

<varExpr> ::=
  <ident>
    
<inputExpr> ::=
  "?"

<funcExpr> ::=
  "func(" (<ident> ("," <ident>)*)? ")" (":" <ident>)? <scopeExpr>

<scopeExpr> ::=
  "{" <program> "}"
```
