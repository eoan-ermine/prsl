#include "prsl/Debug/Logger.hpp"

#include <iostream>
#include <string>

namespace prsl::Errors {

using std::ostream;
using std::string;

void Logger::log(const LogLevel level, int lineNo, const string &msg) {
  counts_[(unsigned int)level]++;
  if (level >= level_) {
    ostream &out = (level == LogLevel::ERROR) ? err_ : out_;
    out << "<source>";
    if (lineNo >= 0) {
      out << ":" << lineNo;
    }
    out << ": ";
    switch (level) {
    case LogLevel::WARNING:
      out << "\u001b[1m\u001b[95mwarning: \u001b[97m";
      break;
    case LogLevel::ERROR:
      out << "\u001b[1m\u001b[91merror: \u001b[97m";
      break;
    default:
      break; // do nothing
    }
    out << msg << "\u001b[0m" << std::endl;
  }
}

void Logger::log(const LogLevel level, const string &msg) {
  log(level, {}, msg);
}

void Logger::error(int lineNo, const string &msg) {
  log(LogLevel::ERROR, lineNo, msg);
}

void Logger::warning(int lineNo, const string &msg) {
  log(LogLevel::WARNING, lineNo, msg);
}

void Logger::info(const string &msg) { log(LogLevel::INFO, msg); }

void Logger::debug(const string &msg) { log(LogLevel::DEBUG, msg); }

int Logger::getDebugCount() const {
  return counts_[(unsigned int)LogLevel::DEBUG];
}

int Logger::getInfoCount() const {
  return counts_[(unsigned int)LogLevel::INFO];
}

int Logger::getWarningCount() const {
  return counts_[(unsigned int)LogLevel::WARNING];
}

int Logger::getErrorCount() const {
  return counts_[(unsigned int)LogLevel::ERROR];
}

void Logger::setLevel(LogLevel level) { level_ = level; }

} // namespace prsl::Errors