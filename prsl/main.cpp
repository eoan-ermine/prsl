#include "prsl/Compiler/Compiler.hpp"
#include "prsl/Compiler/CompilerFlags.hpp"
#include <config.hpp>

#include <boost/program_options.hpp>
#include <iostream>
#include <cstdlib>

namespace fs = std::filesystem;
namespace po = boost::program_options;

void conflicting_options(const po::variables_map &vm, const char *opt1,
                         const char *opt2) {
  if (vm.count(opt1) && !vm[opt1].defaulted() && vm.count(opt2) &&
      !vm[opt2].defaulted())
    throw std::logic_error(std::string("Conflicting options '") + opt1 +
                           "' and '" + opt2 + "'.");
}

int main(int argc, char *argv[]) {
  prsl::Errors::Logger logger;
  po::options_description visible("OPTIONS");
  po::options_description hidden("HIDDEN");

  // clang-format off
  visible.add_options()
    ("help", "produce help message")
    ("version", "Print version information")
    ("parse", "run the parser & semantics stage")
    ("codegen", "produce LLVM IR for given code")
    ("interpret", "interpret given code (default)")
    (",O", po::value<int>()->value_name("<level>"), "Optimization level. [O0, O1, O2, O3]")
    ("filetype", po::value<std::string>()->value_name("<type>"), "Set type of output file. [asm, bc, obj, ll]")
    ("reloc", po::value<std::string>()->value_name("<model>"), "Set relocation model. [default, static, pic]")
    ("target", po::value<std::string>()->value_name("<triple>"), "Target triple for cross compilation.")
    ("no-diagnostics-color", "Do not colorize diagnostics")
    (",o", po::value<std::string>()->value_name("<filename>")->default_value("output"), "Name of the output file")
  ;
  hidden.add_options()
    ("inputs", po::value<std::string>());
  // clang-format on

  po::positional_options_description desc_pos;
  desc_pos.add("inputs", 1);
  po::options_description all;
  all.add(visible).add(hidden);

  po::variables_map vm;
  try {
    po::store(po::command_line_parser(argc, argv)
                  .options(all)
                  .positional(desc_pos)
                  .run(),
              vm);
  } catch (po::error &e) {
    logger.error(PROJECT_NAME, e.what());
    return EXIT_FAILURE;
  }
  po::notify(vm);

  conflicting_options(vm, "parse", "interpret");
  conflicting_options(vm, "parse", "codegen");
  conflicting_options(vm, "codegen", "interpret");

  if (vm.count("help")) {
    std::cout << "OVERVIEW: " << PROJECT_NAME << " LLVM compiler\n"
              << std::endl;
    std::cout << "USAGE: " << PROJECT_NAME << " [options] file\n" << std::endl;
    std::cout << visible << std::endl;
    return EXIT_SUCCESS;
  } else if (vm.count("version")) {
    std::cout << PROJECT_NAME << " version " << PROJECT_VERSION << std::endl;
    std::cout << "Includes: ";
    std::cout << "Boost " << BOOST_VERSION / 100000 << "."
              << BOOST_VERSION / 100 % 1000 << "." << BOOST_VERSION % 100
              << ", ";
    std::cout << "LLVM " << LLVM_VERSION << std::endl;

    return EXIT_SUCCESS;
  } else if (vm.count("inputs")) {
    auto flags = std::make_unique<prsl::Compiler::CompilerFlags>();

    if (vm.count("-O")) {
      int level = vm["-O"].as<int>();
      switch (level) {
      case 0:
        flags->setOptimizationLevel(prsl::Compiler::OptimizationLevel::O0);
        break;
      case 1:
        flags->setOptimizationLevel(prsl::Compiler::OptimizationLevel::O1);
        break;
      case 2:
        flags->setOptimizationLevel(prsl::Compiler::OptimizationLevel::O2);
        break;
      case 3:
        flags->setOptimizationLevel(prsl::Compiler::OptimizationLevel::O3);
        break;
      default:
        logger.error(PROJECT_NAME, "unknown optimization level");
        return EXIT_FAILURE;
      }
    }
    if (vm.count("filetype")) {
      const auto &type = vm["filetype"].as<std::string>();
      if (type == "asm") {
        flags->setFileType(prsl::Compiler::OutputFileType::AsmFile);
      } else if (type == "bc") {
        flags->setFileType(prsl::Compiler::OutputFileType::BitCodeFile);
      } else if (type == "ll") {
        flags->setFileType(prsl::Compiler::OutputFileType::LLVMIRFile);
      } else if (type == "obj") {
        flags->setFileType(prsl::Compiler::OutputFileType::ObjectFile);
      } else {
        logger.error(PROJECT_NAME, "unknown file type");
        return EXIT_FAILURE;
      }
    }
    if (vm.count("reloc")) {
      const auto &model = vm["reloc"].as<std::string>();
      if (model == "pic") {
        flags->setRelocationModel(prsl::Compiler::RelocationModel::PIC);
      } else if (model == "static") {
        flags->setRelocationModel(prsl::Compiler::RelocationModel::STATIC);
      } else {
        flags->setRelocationModel(prsl::Compiler::RelocationModel::DEFAULT);
      }
    }
    if (vm.count("target")) {
      flags->setTargetTriple(vm["target"].as<std::string>());
    }

    // Detect NO_COLOR=1 environment variable
    std::string noColorEnv = []() {
      auto s = std::getenv("NO_COLOR");
      if (!s)
        return std::string{};
      return std::string{s};
    }();
    if (noColorEnv == "1" || vm.count("no-diagnostics-color")) {
      flags->setNoDiagnosticsColor(true);
      logger.setColor(false);
    }

    if (vm.count("-o")) {
      flags->setOutputFile(vm["-o"].as<std::string>());
    }

    auto path = fs::path(vm["inputs"].as<std::string>());
    flags->setExecutionMode(
        vm.count("parse")
            ? prsl::Compiler::ExecutionMode::PARSE
            : (vm.count("codegen") ? prsl::Compiler::ExecutionMode::COMPILE
                                   : prsl::Compiler::ExecutionMode::INTERPRET));

    auto compiler = std::make_unique<prsl::Compiler::Compiler>(logger, flags.get());
    compiler->run(path);
  } else {
    logger.error(PROJECT_NAME, "no input file");
    return EXIT_FAILURE;
  }
}