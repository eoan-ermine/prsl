#pragma once

#include "prsl/AST/NodeTypes.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <variant>

namespace prsl::Evaluator {

class FuncObj;

using FuncObjPtr = std::shared_ptr<FuncObj>;

using PrslObject = std::variant<int, bool, std::nullptr_t, FuncObjPtr>;

bool areEqual(const PrslObject &lhs, const PrslObject &rhs);
std::string toString(const PrslObject &object);
bool isTrue(const PrslObject &object);

class FuncObj {
public:
  explicit FuncObj(const AST::FuncExprPtr &declaration);

  [[nodiscard]] size_t paramsCount() const;
  [[nodiscard]] const AST::FuncExprPtr &getDeclaration() const;

private:
  const AST::FuncExprPtr &declaration;
};

} // namespace prsl::Evaluator