#include "prsl/Compiler/CompilerFlags.hpp"

namespace prsl::Compiler {

void CompilerFlags::setOutputFile(std::string file) { this->outFile = file; }

std::string CompilerFlags::getOutputFile() const { return outFile; }

void CompilerFlags::setTargetTriple(std::string target) {
  this->target = target;
}

std::string CompilerFlags::getTragetTriple() const { return target; }

void CompilerFlags::setFileType(OutputFileType type) { this->type = type; }

OutputFileType CompilerFlags::getFileType() const { return type; }

void CompilerFlags::setOptimizationLevel(OptimizationLevel level) {
  this->level = level;
}

OptimizationLevel CompilerFlags::getOptimizationLevel() const { return level; }

void CompilerFlags::setRelocationModel(RelocationModel model) {
  this->model = model;
}

RelocationModel CompilerFlags::getRelocationModel() const { return model; }

void CompilerFlags::setExecutionMode(ExecutionMode mode) {
  this->executionMode = mode;
}

ExecutionMode CompilerFlags::getExecutionMode() const { return executionMode; }

} // namespace prsl::Compiler