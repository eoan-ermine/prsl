#pragma once

#include "prsl/AST/NodeTypes.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <variant>

namespace prsl::Interpreter {

class FuncObj;

using FuncObjPtr = std::shared_ptr<FuncObj>;

using PrslObject = std::variant<int, bool, std::nullptr_t, FuncObjPtr>;

bool areEqual(const PrslObject &lhs, const PrslObject &rhs) noexcept;
std::string toString(const PrslObject &object);
bool isTrue(const PrslObject &object) noexcept;

class FuncObj {
public:
  explicit FuncObj(const AST::FuncExprPtr &declaration) noexcept;

  [[nodiscard]] size_t paramsCount() const noexcept;
  [[nodiscard]] const AST::FuncExprPtr &getDeclaration() const noexcept;

private:
  const AST::FuncExprPtr &declaration;
};

} // namespace prsl::Interpreter