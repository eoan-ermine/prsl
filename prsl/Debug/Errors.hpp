#pragma once

#include "prsl/Debug/Logger.hpp"
#include "prsl/Parser/Token.hpp"

#include <exception>
#include <string>

using namespace std::string_literals;

namespace prsl::Errors {

template <typename T>
T reportError(Logger &logger, const prsl::Types::Token &token,
              const std::string &message) noexcept {
  logger.error(token.getStartPos(),
               "at '"s + token.toString() + "': "s + message);
  return T{};
}

class ParseError : public std::exception {};

ParseError reportParseError(Logger &logger, const prsl::Types::Token &token,
                            const std::string &message) noexcept;

class RuntimeError : public std::exception {};

RuntimeError reportRuntimeError(Logger &logger, const prsl::Types::Token &token,
                                const std::string &message) noexcept;

} // namespace prsl::Errors
