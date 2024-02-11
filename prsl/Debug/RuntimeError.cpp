#include "prsl/Debug/RuntimeError.hpp"

namespace prsl::Errors {

RuntimeError reportRuntimeError(ErrorReporter &eReporter,
                                const prsl::Types::Token &token,
                                const std::string &message) noexcept {
  eReporter.setError(token.getLine(), token.toString(), ": ", message);
  return RuntimeError();
}

} // namespace prsl::Errors