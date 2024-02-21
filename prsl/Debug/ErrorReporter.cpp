#include "prsl/Debug/ErrorReporter.hpp"

#include <iostream>

namespace prsl::Errors {

PrslStatus ErrorReporter::getStatus() const noexcept { return status; }

void ErrorReporter::printToErr() const noexcept {
  for (const auto &s : errorMessages) {
    std::cerr << s << std::endl;
  }
}

void ErrorReporter::clearErrors() noexcept {
  errorMessages.clear();
  status = PrslStatus::OK;
}

} // namespace prsl::Errors