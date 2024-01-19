#include <iostream>

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

void runFile(std::string_view path) {
  std::ifstream file{path.data()};
  std::string source{std::istream_iterator<char>{file},
                     std::istream_iterator<char>{}};
  prsl::Errors::ErrorReporter eReporter;
  run(eReporter, prsl::Evaluator::Evaluator{eReporter}, source);
}

void runPrompt() {
  prsl::Errors::ErrorReporter eReporter;
  prsl::Evaluator::Evaluator evaluator(eReporter);

  for (std::string line; std::getline(std::cin, line);) {
    run(eReporter, evaluator, line);
  }
}

int main(int argc, char *argv[]) {
  if (argc > 2) {
    std::cout << "Usage: prsl [script]";
    return 64;
  } else if (argc == 2) {
    runFile(argv[0]);
  } else {
    runPrompt();
  }
}