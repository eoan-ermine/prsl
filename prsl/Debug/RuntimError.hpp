#pragma once

#include <exception>
#include <string>

#include "../Types/Token.hpp"
#include "ErrorReporter.hpp"

namespace prsl::Errors {

class RuntimeError : std::exception {};

inline auto reportRuntimeError(ErrorReporter &eReporter,
                               const prsl::Types::Token &token,
                               const std::string &message) -> RuntimeError {
  eReporter.setError(token.getLine(), token.toString(), ": ", message);
  return RuntimeError();
}

} // namespace prsl::Errors