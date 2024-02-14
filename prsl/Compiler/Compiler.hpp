#include "prsl/Compiler/CompilerFlags.hpp"

#include <filesystem>

namespace prsl::Compiler {

namespace fs = std::filesystem;

class Compiler {
public:
  explicit Compiler(CompilerFlags *flags) : flags(flags) {}
  ~Compiler() = default;

  void run(const fs::path &);

private:
  CompilerFlags *flags;
};

} // namespace prsl::Compiler
