#include "prsl/Debug/Errors.hpp"

namespace prsl::Errors {

ParseError reportParseError(Logger &logger, const prsl::Types::Token &token,
                            const std::string &message) noexcept {
  return reportError<ParseError>(logger, token, message);
}

RuntimeError reportRuntimeError(Logger &logger, const prsl::Types::Token &token,
                                const std::string &message) noexcept {
  return reportError<RuntimeError>(logger, token, message);
}

} // namespace prsl::Errors