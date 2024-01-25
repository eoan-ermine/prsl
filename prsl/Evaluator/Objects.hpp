#pragma once

#include <cstddef>
#include <string>
#include <variant>

namespace prsl::Evaluator {

using PrslObject = std::variant<int, bool, std::nullptr_t>;

bool areEqual(const PrslObject &lhs, const PrslObject &rhs);
std::string toString(const PrslObject &object);
bool isTrue(const PrslObject &object);

} // namespace prsl::Evaluator