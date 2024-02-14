#include "prsl/Compiler/Compiler.hpp"
#include "prsl/Compiler/Codegen/Codegen.hpp"
#include "prsl/Compiler/Executor.hpp"
#include "prsl/Debug/ErrorReporter.hpp"
#include "prsl/Debug/RuntimeError.hpp"
#include "prsl/Compiler/Interpreter/Interpreter.hpp"
#include "prsl/Parser/Parser.hpp"
#include "prsl/Parser/Scanner.hpp"
#include "prsl/Semantics/Semantics.hpp"

#include <filesystem>
#include <fstream>

namespace prsl::Compiler {

auto parse(std::string_view source, prsl::Errors::ErrorReporter &eReporter) {
  prsl::Scanner::Scanner scanner(source);
  auto tokens = scanner.tokenize();
  prsl::Parser::Parser parser(tokens, eReporter);
  return parser.parse();
}

auto resolve(const prsl::AST::StmtPtrVariant &stmt,
             prsl::Errors::ErrorReporter &eReporter) {
  prsl::Semantics::Semantics resolver(eReporter);
  resolver.visitStmt(stmt);
}

void Compiler::run(const fs::path &file) {
  auto inputPath = fs::absolute(file);
  auto outputPath = fs::absolute(flags->getOutputFile());
  auto executionMode = flags->getExecutionMode();

  prsl::Errors::ErrorReporter eReporter;
  std::ifstream fstream{inputPath};
  std::ostringstream sstr;
  sstr << fstream.rdbuf();
  auto source = sstr.str();

  std::unique_ptr<Executor> executor;
  if (executionMode == ExecutionMode::COMPILE) {
    executor = std::move(Executor::Create<prsl::Codegen::Codegen>(flags, eReporter));
  } else if (executionMode == ExecutionMode::INTERPRET) {
    executor =
        std::move(Executor::Create<prsl::Interpreter::Interpreter>(flags, eReporter));
  }

  try {
    auto stmt = parse(source, eReporter);
    if (eReporter.getStatus() != prsl::Errors::PrslStatus::OK) {
      eReporter.printToErr();
      return;
    }
    resolve(stmt, eReporter);
    if (eReporter.getStatus() != prsl::Errors::PrslStatus::OK) {
      eReporter.printToErr();
      return;
    }

    if (executionMode != ExecutionMode::PARSE) {
      executor->visitStmt(stmt);
      executor->dump(outputPath);
    }
  } catch (prsl::Errors::RuntimeError &e) {
  }

  if (eReporter.getStatus() != prsl::Errors::PrslStatus::OK) {
    eReporter.printToErr();
  }
}

} // namespace prsl::Compiler