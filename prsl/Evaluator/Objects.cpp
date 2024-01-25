#include "Objects.hpp"

namespace prsl::Evaluator {

bool areEqual(const PrslObject &lhs, const PrslObject &rhs) {
  if (lhs.index() == rhs.index()) {
    switch (lhs.index()) {
    case 0:
      return std::get<int>(lhs) == std::get<int>(rhs);
    case 1:
      return std::get<bool>(lhs) == std::get<int>(rhs);
    case 2:
      return true;
    }
  }
  return false;
}

std::string toString(const PrslObject &object) {
  switch (object.index()) {
  case 0:
    return std::to_string(std::get<int>(object));
  case 1:
    return std::get<bool>(object) == true ? "true" : "false";
  }
  return "";
}

bool isTrue(const PrslObject &object) {
  switch (object.index()) {
  case 0:
    return std::get<int>(object) != 0;
  case 1:
    return std::get<bool>(object) == true;
  }
  return false;
}

} // namespace prsl::Evaluator