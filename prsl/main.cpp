#include <iostream>

#include "prsl/Codegen/Codegen.hpp"
#include "prsl/Debug/ErrorReporter.hpp"
#include "prsl/Evaluator/Evaluator.hpp"
#include "prsl/Parser/parser.hpp"
#include "prsl/Scanner/scanner.hpp"
#include <fstream>

#include <boost/program_options.hpp>
#include <string_view>

enum class InputMode { REPL, FILE };

template <typename T>
auto execute(std::string_view inputFilename, InputMode inputMode,
             std::string_view outputFilename) {
  prsl::Errors::ErrorReporter eReporter;
  T executor(eReporter);

  auto executeStmts = [&](std::string_view source) {
    prsl::Scanner::Scanner scanner(source);
    auto tokens = scanner.tokenize();
    prsl::Parser::Parser parser(tokens, eReporter);
    auto statements = parser.parse();

    if (eReporter.getStatus() != prsl::Errors::PrslStatus::OK) {
      eReporter.printToErr();
      return;
    }

    executor.executeStmts(statements);
  };

  if (inputMode == InputMode::FILE) {
    std::ifstream file{inputFilename.data()};
    std::ostringstream sstr;
    sstr << file.rdbuf();
    auto source = sstr.str();
    executeStmts(source);
    executor.dump(outputFilename);
  } else {
    std::string line;
    while (std::getline(std::cin, line)) {
      executeStmts(line);
    }
    executor.dump(outputFilename);
  }
}

namespace po = boost::program_options;

void conflicting_options(const po::variables_map &vm, const char *opt1,
                         const char *opt2) {
  if (vm.count(opt1) && !vm[opt1].defaulted() && vm.count(opt2) &&
      !vm[opt2].defaulted())
    throw std::logic_error(std::string("Conflicting options '") + opt1 +
                           "' and '" + opt2 + "'.");
}

int main(int argc, char *argv[]) try {
  po::options_description desc("Usage: prsl [INPUT-FILE] ...");
  // clang-format off
  desc.add_options()
    ("help", "produce help message")
    ("codegen", "produce LLVM IR for given code")
    ("interpret", "interpret given code (default)")
    ("input-file", po::value<std::string>(), "Input file")
    ("output-file", po::value<std::string>()->default_value(std::string{"output"}), "Output file")
  ;
  // clang-format on

  po::positional_options_description desc_pos;
  desc_pos.add("input-file", 1);

  po::variables_map vm;
  auto options = po::command_line_parser(argc, argv)
                     .options(desc)
                     .positional(desc_pos)
                     .run();
  po::store(options, vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << '\n';
    return EXIT_SUCCESS;
  }

  conflicting_options(vm, "codegen", "interpret");

  std::string inputFile;
  InputMode inputMode =
      vm.count("input-file") ? InputMode::FILE : InputMode::REPL;
  inputFile =
      inputMode == InputMode::FILE ? vm["input-file"].as<std::string>() : "";
  auto outputFile = vm["output-file"].as<std::string>();

  if (vm.count("codegen")) {
    execute<prsl::Codegen::Codegen>(inputFile, inputMode, outputFile);
  } else {
    execute<prsl::Evaluator::Evaluator>(inputFile, inputMode, outputFile);
  }
} catch (const std::exception &e) {
  std::cout << "prsl: error: " << e.what() << '\n';
  return EXIT_FAILURE;
}