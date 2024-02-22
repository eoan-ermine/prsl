#include "prsl/Compiler/CompilerFlags.hpp"
#include "prsl/Debug/Logger.hpp"

#include <filesystem>

namespace prsl::Compiler {

namespace fs = std::filesystem;

class Compiler {
public:
  explicit Compiler(Errors::Logger &logger, CompilerFlags *flags)
      : logger(logger), flags(flags) {}
  ~Compiler() = default;

  void run(const fs::path &);

private:
  Errors::Logger &logger;
  CompilerFlags *flags;
};

} // namespace prsl::Compiler
