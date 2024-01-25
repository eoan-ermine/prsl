#pragma once

#include <cstddef>
#include <string>
#include <variant>

namespace prsl::Evaluator {

using PrslObject = std::variant<int, bool, std::nullptr_t>;

inline auto areEqual(const PrslObject &lhs, const PrslObject &rhs) -> bool {
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

inline auto toString(const PrslObject &object) -> std::string {
  switch (object.index()) {
  case 0:
    return std::to_string(std::get<int>(object));
  case 1:
    return std::get<bool>(object) == true ? "true" : "false";
  }
  return "";
}

inline auto isTrue(const PrslObject &object) -> bool {
  switch (object.index()) {
  case 0:
    return std::get<int>(object) != 0;
  case 1:
    return std::get<bool>(object) == true;
  }
  return false;
}

} // namespace prsl::Evaluator