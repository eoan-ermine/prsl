#pragma once

#include <cstddef>
#include <exception>
#include <memory>
#include <unordered_map>

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

  void assign(size_t varNameHash, VarValue object) {
    auto iter = objects.find(varNameHash);
    if (iter != objects.end()) {
      objects.insert_or_assign(varNameHash, object);
      return;
    }

    if (parentEnv != nullptr)
      return parentEnv->assign(varNameHash, std::move(object));

    throw UndefVarAccess{};
  }

  void defineOrAssign(size_t varNameHash, VarValue object) {
    if (!contains(varNameHash)) {
      objects.insert_or_assign(varNameHash, object);
      return;
    }
    assign(varNameHash, object);
  }

  VarValue get(size_t varNameHash) {
    auto it = objects.find(varNameHash);
    if (it != objects.end()) {
      return it->second;
    }

    if (parentEnv != nullptr)
      return parentEnv->get(varNameHash);

    throw UndefVarAccess{};
  }

  bool contains(size_t varNameHash) {
    if (parentEnv != nullptr)
      return parentEnv->contains(varNameHash);
    return objects.contains(varNameHash);
  }

  EnvironmentPtr getParentEnv() { return parentEnv; }

  bool isGlobal() { return parentEnv == nullptr; }

private:
  std::unordered_map<size_t, VarValue> objects;
  EnvironmentPtr parentEnv = nullptr;
};

template <typename VarValue> class EnvironmentManager {
public:
  explicit EnvironmentManager(ErrorReporter &eReporter)
      : eReporter(eReporter),
        curEnv(std::make_shared<Environment<VarValue>>(nullptr)) {}

  template <typename F> void withNewEnviron(F &&action) {
    auto environToRestore = curEnv;
    createNewEnv();
    action();
    discardEnvsTill(environToRestore);
  }

  void assign(const Types::Token &token, VarValue object) {
    try {
      curEnv->assign(hasher(token.getLexeme()), std::move(object));
    } catch (const UndefVarAccess &e) {
      throw Errors::reportRuntimeError(eReporter, token,
                                       "Attempt to access an undef variable");
    }
  }

  void define(std::string_view str, VarValue object) {
    curEnv->defineOrAssign(hasher(str), std::move(object));
  }

  void define(const Types::Token &token, VarValue object) {
    curEnv->defineOrAssign(hasher(token.getLexeme()), std::move(object));
  }

  VarValue get(const Types::Token &token) {
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

  bool contains(const Types::Token &token) {
    return curEnv->contains(hasher(token.getLexeme()));
  }

private:
  void createNewEnv() {
    curEnv = std::make_shared<Environment<VarValue>>(curEnv);
  }

  void discardEnvsTill(
      const Environment<VarValue>::EnvironmentPtr &environToRestore) {
    while (!curEnv->isGlobal() && curEnv.get() != environToRestore.get())
      curEnv = curEnv->getParentEnv();
  }

private:
  ErrorReporter &eReporter;
  Environment<VarValue>::EnvironmentPtr curEnv;
  std::hash<std::string_view> hasher;
};

}; // namespace prsl::Evaluator