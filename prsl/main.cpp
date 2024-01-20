#include <iostream>

#include "prsl/Codegen/Codegen.hpp"
#include "prsl/Debug/ErrorReporter.hpp"
#include "prsl/Evaluator/Evaluator.hpp"
#include "prsl/Parser/parser.hpp"
#include "prsl/Scanner/scanner.hpp"
#include <fstream>
#include <iterator>

void run(prsl::Errors::ErrorReporter eReporter,
         prsl::Evaluator::Evaluator evaluator, std::string_view source) {
  prsl::Scanner::Scanner scanner(source);
  auto tokens = scanner.tokenize();
  prsl::Parser::Parser parser(tokens, eReporter);
  auto statements = parser.parse();

  if (eReporter.getStatus() != prsl::Errors::PrslStatus::OK) {
    eReporter.printToErr();
  }
  evaluator.evaluateStmts(statements);
}

void codegen(prsl::Errors::ErrorReporter eReporter,
             prsl::Codegen::Codegen &codegen, std::string_view source) {
  prsl::Scanner::Scanner scanner(source);
  auto tokens = scanner.tokenize();
  prsl::Parser::Parser parser(tokens, eReporter);
  auto statements = parser.parse();

  if (eReporter.getStatus() != prsl::Errors::PrslStatus::OK) {
    eReporter.printToErr();
    return;
  }

  codegen.codegenStmts(statements);
  codegen.print();
}

void runFile(std::string_view path, bool codegen_ = false) {
  std::ifstream file{path.data()};
  std::string source{std::istream_iterator<char>{file},
                     std::istream_iterator<char>{}};
  prsl::Errors::ErrorReporter eReporter;
  if (codegen_) {
    prsl::Codegen::Codegen codegenObj{eReporter};
    codegen(eReporter, codegenObj, source);
  } else
    run(eReporter, prsl::Evaluator::Evaluator{eReporter}, source);
}

void runPrompt(bool codegen_ = false) {
  prsl::Errors::ErrorReporter eReporter;
  prsl::Evaluator::Evaluator evaluator(eReporter);
  prsl::Codegen::Codegen codegenObj{eReporter};

  for (std::string line; std::getline(std::cin, line);) {
    if (codegen_)
      codegen(eReporter, codegenObj, line);
    else
      run(eReporter, evaluator, line);
  }
}

int main(int argc, char *argv[]) {
  if (argc > 3) {
    std::cout << "Usage: prsl [script]";
    return 64;
  } else if (argc == 2) {
    if (std::string(argv[1]) == "--codegen")
      runPrompt(true);
    else
      runFile(argv[0]);
  } else if (argc == 3) {
    runFile(argv[2], true);
  } else {
    runPrompt();
  }
}