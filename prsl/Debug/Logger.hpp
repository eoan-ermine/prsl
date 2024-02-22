#pragma once

#include "prsl/Utils/Utils.hpp"

#include <array>
#include <iostream>
#include <string>

namespace prsl::Errors {

using std::cerr;
using std::cout;
using std::ostream;
using std::string;

enum class LogLevel { DEBUG = 1, INFO = 2, WARNING = 3, ERROR = 4, QUIET = 5 };

class Logger {
public:
  Logger() : Logger(LogLevel::ERROR, cout, cerr) {}
  Logger(LogLevel level, ostream &out) : Logger(level, out, out){};
  Logger(LogLevel level, ostream &out, ostream &err);
  Logger(const Logger &) = delete;
  Logger &operator=(const LogLevel &) = delete;
  ~Logger() = default;

  void error(const Utils::FilePos &pos, const string &msg);
  void error(const string &fileName, const string &msg);
  void warning(const Utils::FilePos &pos, const string &msg);
  void warning(const string &fileName, const string &msg);
  void info(const string &msg);
  void debug(const string &msg);

  [[nodiscard]] int getDebugCount() const;
  [[nodiscard]] int getInfoCount() const;
  [[nodiscard]] int getWarningCount() const;
  [[nodiscard]] int getErrorCount() const;

  void setLevel(LogLevel level);
  void setColor(bool color);

private:
  void log(LogLevel level, std::string_view fileName, int line, int col,
           const string &msg);
  void log(LogLevel level, std::string_view fileName, const string &msg);
  void log(LogLevel level, const string &msg);

private:
  LogLevel level;
  ostream &out, &err;
  std::array<int, (size_t)LogLevel::QUIET> counts;
  bool color;
  std::array<const char *, (size_t)LogLevel::QUIET> messages;
  std::array<std::pair<const char *, const char *>, (size_t)LogLevel::QUIET>
      colors;
};

} // namespace prsl::Errors