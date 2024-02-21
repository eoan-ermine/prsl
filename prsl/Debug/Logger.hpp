#pragma once

#include <string>
#include <iostream>

namespace prsl::Errors {

using std::cerr;
using std::cout;
using std::ostream;
using std::string;

enum class LogLevel { DEBUG = 1, INFO = 2, WARNING = 3, ERROR = 4, QUIET = 5 };

class Logger {
public:
    Logger() : Logger(LogLevel::ERROR, cout, cerr) {};
    Logger(LogLevel level, ostream &out) : Logger(level, out, out) {};
    Logger(LogLevel level, ostream &out, ostream &err) : level_(level), out_(out), err_(err), counts_() {};
    Logger(const Logger &) = delete;
    Logger& operator=(const LogLevel&) = delete;
    ~Logger() = default;

    void error(int line, const string &msg);
    void warning(int line, const string &msg);
    void info(const string &msg);
    void debug(const string &msg);

    [[nodiscard]] int getDebugCount() const;
    [[nodiscard]] int getInfoCount() const;
    [[nodiscard]] int getWarningCount() const;
    [[nodiscard]] int getErrorCount() const;

    void setLevel(LogLevel level);
private:
    void log(LogLevel level, int lineNo, const string &msg);
    void log(LogLevel level, const string &msg);
private:
    LogLevel level_;
    ostream &out_, &err_;
    int counts_[(unsigned int) LogLevel::QUIET];
};

}