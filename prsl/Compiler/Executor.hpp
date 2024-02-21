#include "prsl/AST/NodeTypes.hpp"
#include "prsl/Compiler/CompilerFlags.hpp"
#include "prsl/Debug/Logger.hpp"
#include <filesystem>

namespace prsl::Compiler {

class Executor {
public:
  template <typename T>
  static std::unique_ptr<Executor> Create(CompilerFlags *flags,
                                          Errors::Logger &logger) {
    return std::make_unique<Executor>(
        std::make_unique<holder<T>>(flags, logger));
  }

  template <typename T>
  explicit Executor(std::unique_ptr<T> ptr) : ptr(std::move(ptr)) {}

  void visitStmt(const AST::StmtPtrVariant &stmt) { ptr->visitStmt(stmt); }

  void dump(const std::filesystem::path &path) const { ptr->dump(path); }

private:
  struct base_holder {
    virtual ~base_holder() {}
    virtual void visitStmt(const AST::StmtPtrVariant &stmt) = 0;
    virtual void dump(const std::filesystem::path &path) const = 0;
  };

  template <typename T> struct holder : base_holder {
    T executor;

    holder(CompilerFlags *flags, Errors::Logger &logger)
        : executor(flags, logger) {}

    void visitStmt(const AST::StmtPtrVariant &stmt) override {
      executor.visitStmt(stmt);
    }

    void dump(const std::filesystem::path &path) const override {
      executor.dump(path);
    }
  };

  std::unique_ptr<base_holder> ptr;
};

} // namespace prsl::Compiler