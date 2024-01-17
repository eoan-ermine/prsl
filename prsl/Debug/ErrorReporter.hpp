#pragma once

#pragma once

#include <iostream>
#include <string>
#include <vector>

namespace prsl::Errors {

enum struct PrslStatus { OK, ERROR };

class ErrorReporter {
public:
  void clearErrors() {
    errorMessages.clear();
    status = PrslStatus::OK;
  }
  auto getStatus() -> PrslStatus { return status; }
  void printToErr() {
    for (auto &s : errorMessages) {
      std::cerr << s << std::endl;
    }
  }
  template <typename... Args>
  void setError(int line, Args&&... params) {
    errorMessages.emplace_back("[Line " + std::to_string(line) +
                               "] Error: " + (params + ...));
    status = PrslStatus::ERROR;
  }

private:
  std::vector<std::string> errorMessages;
  PrslStatus status = PrslStatus::OK;
};

} // namespace prsl::ErrorsAndDebug