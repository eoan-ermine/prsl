#pragma once

#include <cstddef>
#include <exception>
#include <memory>
#include <unordered_map>
#include <variant>

#include "prsl/Debug/ErrorReporter.hpp"
#include "prsl/Debug/RuntimeError.hpp"
#include "prsl/Types/Token.hpp"

namespace prsl::Evaluator {

using prsl::Errors::ErrorReporter;

namespace {

class UndefVarAccess : public std::exception {};
class UninitVarAccess : public std::exception {};

} // namespace

template <typename VarValue>
class Environment : std::enable_shared_from_this<Environment<VarValue>> {
public:
  using EnvironmentPtr = std::shared_ptr<Environment>;

  explicit Environment(EnvironmentPtr parentEnv)
      : parentEnv(std::move(parentEnv)) {}

  auto assign(size_t varNameHash, VarValue object) -> bool {
    auto iter = objects.find(varNameHash);
    if (iter != objects.end()) {
      objects.insert_or_assign(varNameHash, object);
      return true;
    }

    throw UndefVarAccess{};
  }

  void define(size_t varNameHash, VarValue object) {
    objects.insert_or_assign(varNameHash, object);
  }

  auto get(size_t varNameHash) -> VarValue {
    auto it = objects.find(varNameHash);
    if (it != objects.end()) {
      // TODO: Universal check for uninit
      return it->second;
    }

    if (parentEnv != nullptr)
      return parentEnv->get(varNameHash);

    throw UndefVarAccess{};
  }

  auto contains(size_t varNameHash) -> bool {
    return objects.contains(varNameHash);
  }

  auto getParentEnv() -> EnvironmentPtr { return parentEnv; }

  auto isGlobal() -> bool { return parentEnv == nullptr; }

private:
  std::unordered_map<size_t, VarValue> objects;
  EnvironmentPtr parentEnv = nullptr;
};

template <typename VarValue> class EnvironmentManager {
public:
  explicit EnvironmentManager(ErrorReporter &eReporter)
      : eReporter(eReporter),
        curEnv(std::make_shared<Environment<VarValue>>(nullptr)) {}

  void createNewEnviron() {
    curEnv = std::make_shared<Environment<VarValue>>(curEnv);
  }

  void discardEnvironsTill(
      const Environment<VarValue>::EnvironmentPtr &environToRestore) {
    while (!curEnv->isGlobal() && curEnv.get() != environToRestore.get())
      curEnv = curEnv->getParentEnv();
  }

  void assign(const Types::Token &token, VarValue object) {
    try {
      curEnv->assign(hasher(token.getLexeme()), std::move(object));
    } catch (const UndefVarAccess &e) {
      throw Errors::reportRuntimeError(eReporter, token,
                                       "Attempt to access an uninit variable");
    }
  }

  void define(std::string_view str, VarValue object) {
    curEnv->define(hasher(str), std::move(object));
  }

  void define(const Types::Token &token, VarValue object) {
    curEnv->define(hasher(token.getLexeme()), std::move(object));
  }

  auto get(const Types::Token &token) -> VarValue {
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

  auto contains(const Types::Token &token) -> bool {
    return curEnv->contains(hasher(token.getLexeme()));
  }

  auto getCurEnv() -> Environment<VarValue>::EnvironmentPtr { return curEnv; }

  void setCurEnv(Environment<VarValue>::EnvironmentPtr newCur) {
    curEnv = std::move(newCur);
  }

private:
  ErrorReporter &eReporter;
  Environment<VarValue>::EnvironmentPtr curEnv;
  std::hash<std::string_view> hasher;
};

}; // namespace prsl::Evaluator