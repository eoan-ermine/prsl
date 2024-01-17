#pragma once

#pragma once

#include <iostream>
#include <string>
#include <vector>

namespace prsl::ErrorsAndDebug {

enum struct LoxStatus { OK, ERROR };

class ErrorReporter {
public:
  void clearErrors() {
    errorMessages.clear();
    status = LoxStatus::OK;
  }
  auto getStatus() -> LoxStatus { return status; }
  void printToErr() {
    for (auto &s : errorMessages) {
      std::cerr << s << std::endl;
    }
  }
  template <typename... Args>
  void setError(int line, Args&&... params) {
    errorMessages.emplace_back("[Line " + std::to_string(line) +
                               "] Error: " + (params + ...));
    status = LoxStatus::ERROR;
  }

private:
  std::vector<std::string> errorMessages;
  LoxStatus status = LoxStatus::OK;
};

} // namespace prsl::ErrorsAndDebug