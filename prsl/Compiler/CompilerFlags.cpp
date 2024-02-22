#include "prsl/Compiler/CompilerFlags.hpp"

namespace prsl::Compiler {

void CompilerFlags::setOutputFile(std::string file) {
  this->outFile = std::move(file);
}

std::string CompilerFlags::getOutputFile() const { return outFile; }

void CompilerFlags::setTargetTriple(std::string target) {
  this->target = std::move(target);
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

void CompilerFlags::setNoDiagnosticsColor(bool flag) {
  this->noDiagnosticsColor = flag;
}

bool CompilerFlags::getNoDiagnosticsColor() const { return noDiagnosticsColor; }

} // namespace prsl::Compiler