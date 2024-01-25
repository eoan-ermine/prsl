#pragma once

#include <sstream>
#include <string>
#include <vector>

namespace prsl::Errors {

enum struct PrslStatus { OK, ERROR };

class ErrorReporter {
public:
  PrslStatus getStatus();

  void printToErr();
  void clearErrors();
  template <typename... Args> void setError(int line, Args &&...params) {
    std::ostringstream ss;
    ((ss << std::forward<Args>(params)), ...);
    errorMessages.emplace_back("[Line " + std::to_string(line) +
                               "] Error: " + ss.str());
    status = PrslStatus::ERROR;
  }

private:
  std::vector<std::string> errorMessages;
  PrslStatus status = PrslStatus::OK;
};

} // namespace prsl::Errors