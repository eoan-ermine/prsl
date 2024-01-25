#include "ErrorReporter.hpp"

#include <iostream>

namespace prsl::Errors {

PrslStatus ErrorReporter::getStatus() { return status; }

void ErrorReporter::printToErr() {
  for (auto &s : errorMessages) {
    std::cerr << s << std::endl;
  }
}

void ErrorReporter::clearErrors() {
  errorMessages.clear();
  status = PrslStatus::OK;
}

} // namespace prsl::Errors