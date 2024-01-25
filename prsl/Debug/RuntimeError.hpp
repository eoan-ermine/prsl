#pragma once

#include <exception>
#include <string>

#include "../Types/Token.hpp"
#include "ErrorReporter.hpp"

namespace prsl::Errors {

class RuntimeError : std::exception {};

RuntimeError reportRuntimeError(ErrorReporter &eReporter,
                                const prsl::Types::Token &token,
                                const std::string &message);
} // namespace prsl::Errors