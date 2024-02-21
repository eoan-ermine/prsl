#pragma once

#include "prsl/Debug/ErrorReporter.hpp"
#include "prsl/Parser/Token.hpp"

#include <exception>
#include <string>

namespace prsl::Errors {

class RuntimeError : public std::exception {};

RuntimeError reportRuntimeError(ErrorReporter &eReporter,
                                const prsl::Types::Token &token,
                                const std::string &message) noexcept;
} // namespace prsl::Errors