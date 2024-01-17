#pragma once

#include <exception>
#include <string>

#include "ErrorReporter.hpp"
#include "../Types/Token.hpp"

namespace prsl::Errors {

class RuntimeError : std::exception {};

auto reportRuntimeError(ErrorReporter& eReporter, const Token& token,
                        const std::string& message) -> RuntimeError {
  eReporter.setError(token.getLine(), token.getLexeme(), ": ", message);
  return RuntimeError();
}

}