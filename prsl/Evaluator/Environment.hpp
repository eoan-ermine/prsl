#pragma once

#include <cstddef>
#include <exception>
#include <memory>
#include <unordered_map>
#include <variant>

#include "prsl/Debug/ErrorReporter.hpp"
#include "prsl/Debug/RuntimError.hpp"
#include "prsl/Evaluator/Objects.hpp"
#include "prsl/Types/Token.hpp"

namespace prsl::Evaluator {

using prsl::Errors::ErrorReporter;

namespace {

class UndefVarAccess : public std::exception {};
class UninitVarAccess : public std::exception {};

} // namespace

class Environment : std::enable_shared_from_this<Environment> {
public:
  using EnvironmentPtr = std::shared_ptr<Environment>;

  explicit Environment(EnvironmentPtr parentEnv)
      : parentEnv(std::move(parentEnv)) {}

  auto assign(size_t varNameHash, PrslObject object) -> bool {
    auto iter = objects.find(varNameHash);
    if (iter != objects.end()) {
      objects.insert_or_assign(varNameHash, object);
      return true;
    }

    throw UndefVarAccess{};
  }

  void define(size_t varNameHash, PrslObject object) {
    objects.insert_or_assign(varNameHash, object);
  }

  auto get(size_t varNameHash) -> PrslObject {
    auto it = objects.find(varNameHash);
    if (it != objects.end()) {
      if (std::holds_alternative<std::nullptr_t>(it->second))
        throw UninitVarAccess{};
      return it->second;
    }

    if (parentEnv != nullptr)
      return parentEnv->get(varNameHash);

    throw UndefVarAccess{};
  }

  auto getParentEnv() -> EnvironmentPtr { return parentEnv; }

  auto isGlobal() -> bool { return parentEnv == nullptr; }

private:
  std::unordered_map<size_t, PrslObject> objects;
  EnvironmentPtr parentEnv = nullptr;
};

class EnvironmentManager {
public:
  explicit EnvironmentManager(ErrorReporter &eReporter)
      : eReporter(eReporter), curEnv(std::make_shared<Environment>(nullptr)) {}

  void createNewEnviron() { curEnv = std::make_shared<Environment>(curEnv); }

  void
  discardEnvironsTill(const Environment::EnvironmentPtr &environToRestore) {
    while (!curEnv->isGlobal() && curEnv.get() != environToRestore.get())
      curEnv = curEnv->getParentEnv();
  }

  void assign(const Types::Token &token, PrslObject object) {
    try {
      curEnv->assign(hasher(token.getLexeme()), std::move(object));
    } catch (const UndefVarAccess &e) {
      throw Errors::reportRuntimeError(eReporter, token,
                                       "Attempt to access an uninit variable");
    }
  }

  void define(std::string_view str, PrslObject object) {
    curEnv->define(hasher(str), std::move(object));
  }

  void define(const Types::Token &token, PrslObject object) {
    curEnv->define(hasher(token.getLexeme()), std::move(object));
  }

  auto get(const Types::Token &token) -> PrslObject {
    try {
      return curEnv->get(hasher(token.getLexeme()));
    } catch (const UndefVarAccess &e) {
      throw Errors::reportRuntimeError(eReporter, token,
                                       "Attempt to access an undef variable");
    } catch (const UninitVarAccess &e) {
      throw Errors::reportRuntimeError(eReporter, token,
                                       "Attempt to access an uninit variable");
    }
  }

  auto getCurEnv() -> Environment::EnvironmentPtr { return curEnv; }

  void setCurEnv(Environment::EnvironmentPtr newCur) {
    curEnv = std::move(newCur);
  }

private:
  ErrorReporter &eReporter;
  Environment::EnvironmentPtr curEnv;
  std::hash<std::string_view> hasher;
};

}; // namespace prsl::Evaluator