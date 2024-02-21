#include "prsl/Compiler/Compiler.hpp"
#include "prsl/Compiler/Codegen/Codegen.hpp"
#include "prsl/Compiler/Executor.hpp"
#include "prsl/Compiler/Interpreter/Interpreter.hpp"
#include "prsl/Debug/Logger.hpp"
#include "prsl/Debug/Errors.hpp"
#include "prsl/Parser/Parser.hpp"
#include "prsl/Parser/Scanner.hpp"
#include "prsl/Semantics/Semantics.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace prsl::Compiler {

auto parse(std::string_view source, prsl::Errors::Logger &logger) {
  prsl::Scanner::Scanner scanner(source);
  auto tokens = scanner.tokenize();
  prsl::Parser::Parser parser(tokens, logger);
  return parser.parse();
}

auto resolve(const prsl::AST::StmtPtrVariant &stmt,
             prsl::Errors::Logger &logger) {
  prsl::Semantics::Semantics resolver(logger);
  resolver.visitStmt(stmt);
}

void Compiler::run(const fs::path &file) {
  auto inputPath = fs::absolute(file);
  auto executionMode = flags->getExecutionMode();

  std::ifstream fstream{inputPath};
  std::ostringstream sstr;
  sstr << fstream.rdbuf();
  auto source = sstr.str();

  std::unique_ptr<Executor> executor;
  if (executionMode == ExecutionMode::COMPILE) {
    executor =
        std::move(Executor::Create<prsl::Codegen::Codegen>(flags, logger));
  } else if (executionMode == ExecutionMode::INTERPRET) {
    executor = std::move(
        Executor::Create<prsl::Interpreter::Interpreter>(flags, logger));
  }

  try {
    auto stmt = parse(source, logger);
    if (logger.getErrorCount()) {
      return;
    }
    resolve(stmt, logger);
    if (logger.getErrorCount()) {
      return;
    }

    if (executionMode != ExecutionMode::PARSE) {
      auto outputPath = fs::absolute(flags->getOutputFile());
      executor->visitStmt(stmt);
      executor->dump(outputPath);
    }
  } catch (const prsl::Errors::RuntimeError &e) {
    std::ignore = e;
  }
}

} // namespace prsl::Compiler