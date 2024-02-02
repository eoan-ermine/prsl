#include "Objects.hpp"

#include "prsl/Utils/Utils.hpp"

namespace prsl::Evaluator {

FuncObj::FuncObj(const AST::FuncExprPtr &declaration)
    : declaration(declaration) {}

size_t FuncObj::paramsCount() const { return declaration->parameters.size(); }

const AST::FuncExprPtr &FuncObj::getDeclaration() const { return declaration; }

bool areEqual(const PrslObject &lhs, const PrslObject &rhs) {
  return std::visit<bool>(
      Utils::select{
          [](const int &lhs, const int &rhs) { return lhs == rhs; },
          [](const bool &lhs, const bool &rhs) { return lhs == rhs; },
          [](const std::nullptr_t &, const std::nullptr_t &) { return true; },
          [](const auto &, const auto &) { return false; },
      },
      lhs, rhs);
}

std::string toString(const PrslObject &object) {
  return std::visit<std::string>(
      Utils::select{
          [](const int &val) { return std::to_string(val); },
          [](const bool &val) { return val ? "1" : "0"; },
          [](const std::nullptr_t &) { return "nil"; },
          [](const auto &) { return ""; },
      },
      object);
}

bool isTrue(const PrslObject &object) {
  return std::visit<bool>(Utils::select{
                              [](const int &val) { return val != 0; },
                              [](const bool &val) { return val == true; },
                              [](const auto &) { return false; },
                          },
                          object);
}

} // namespace prsl::Evaluator