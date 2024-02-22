#include "prsl/Debug/Logger.hpp"

#include <iostream>
#include <string>

namespace prsl::Errors {

using std::ostream;
using std::string;

Logger::Logger(LogLevel level, ostream &out, ostream &err)
    : level(level), out(out), err(err), counts(), color(true) {
  messages[(size_t)LogLevel::WARNING] = "warning: ";
  messages[(size_t)LogLevel::ERROR] = "error: ";
  colors[(size_t)LogLevel::WARNING] = {"\u001b[1m\u001b[95m", "\u001b[97m"};
  colors[(size_t)LogLevel::ERROR] = {"\u001b[1m\u001b[91m", "\u001b[97m"};
};

void Logger::log(const LogLevel logLevel, std::string_view filename, int line,
                 int col, const string &msg) {
  counts[(unsigned int)level]++;
  if (logLevel >= level) {
    ostream &outStream = (level == LogLevel::ERROR) ? err : out;
    if (!filename.empty()) {
      outStream << filename;
      if (line >= 0) {
        outStream << ":" << line;
        if (col >= 0) {
          outStream << ":" << col;
        }
      }
      outStream << ": ";
    }

    outStream << (color ? colors[(size_t)level].first : "")
              << messages[(size_t)level]
              << (color ? colors[(size_t)level].second : "");
    outStream << msg << (color ? "\u001b[0m" : "") << std::endl;
  }
}

void Logger::log(const LogLevel logLevel, std::string_view fileName,
                 const string &msg) {
  log(logLevel, fileName, -1, -1, msg);
}

void Logger::log(const LogLevel logLevel, const string &msg) {
  log(logLevel, {}, msg);
}

void Logger::error(const Utils::FilePos &pos, const string &msg) {
  log(LogLevel::ERROR, pos.filename, pos.line, pos.col, msg);
}

void Logger::error(const string &fileName, const string &msg) {
  if (fileName.empty()) {
    log(LogLevel::ERROR, "prsl", msg);
  } else {
    log(LogLevel::ERROR, fileName, msg);
  }
}

void Logger::warning(const Utils::FilePos &pos, const string &msg) {
  log(LogLevel::WARNING, pos.filename, pos.line, pos.col, msg);
}

void Logger::warning(const string &fileName, const string &msg) {
  if (fileName.empty()) {
    log(LogLevel::WARNING, "prsl", msg);
  } else {
    log(LogLevel::WARNING, fileName, msg);
  }
}

void Logger::info(const string &msg) { log(LogLevel::INFO, msg); }

void Logger::debug(const string &msg) { log(LogLevel::DEBUG, msg); }

int Logger::getDebugCount() const {
  return this->counts[(unsigned int)LogLevel::DEBUG];
}

int Logger::getInfoCount() const {
  return this->counts[(unsigned int)LogLevel::INFO];
}

int Logger::getWarningCount() const {
  return this->counts[(unsigned int)LogLevel::WARNING];
}

int Logger::getErrorCount() const {
  return this->counts[(unsigned int)LogLevel::ERROR];
}

void Logger::setLevel(LogLevel level) { this->level = level; }

void Logger::setColor(bool color) { this->color = color; }

} // namespace prsl::Errors