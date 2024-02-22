#pragma once

#include <string>

namespace prsl::Compiler {

enum class OutputFileType { AsmFile, BitCodeFile, LLVMIRFile, ObjectFile };

enum class OptimizationLevel { O0, O1, O2, O3 };

enum class RelocationModel { DEFAULT, STATIC, PIC };

enum class ExecutionMode { PARSE, COMPILE, INTERPRET };

class CompilerFlags {
public:
  CompilerFlags()
      : type(OutputFileType::ObjectFile), level(OptimizationLevel::O0),
        model(RelocationModel::DEFAULT), executionMode(ExecutionMode::PARSE),
        noDiagnosticsColor(false){};
  ~CompilerFlags() = default;

  void setOutputFile(std::string file);
  [[nodiscard]] std::string getOutputFile() const;

  void setTargetTriple(std::string target);
  [[nodiscard]] std::string getTragetTriple() const;

  void setFileType(OutputFileType type);
  [[nodiscard]] OutputFileType getFileType() const;

  void setOptimizationLevel(OptimizationLevel level);
  [[nodiscard]] OptimizationLevel getOptimizationLevel() const;

  void setRelocationModel(RelocationModel model);
  [[nodiscard]] RelocationModel getRelocationModel() const;

  void setExecutionMode(ExecutionMode mode);
  [[nodiscard]] ExecutionMode getExecutionMode() const;

  void setNoDiagnosticsColor(bool flag);
  [[nodiscard]] bool getNoDiagnosticsColor() const;

private:
  std::string outFile;
  std::string target;
  OutputFileType type;
  OptimizationLevel level;
  RelocationModel model;
  ExecutionMode executionMode;
  bool noDiagnosticsColor;
};

} // namespace prsl::Compiler