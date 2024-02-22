#pragma once

#include "prsl/Debug/Errors.hpp"
#include "prsl/Debug/Logger.hpp"
#include "prsl/Parser/Token.hpp"

#include <exception>
#include <memory>
#include <unordered_map>

namespace prsl::Types {

using prsl::Errors::Logger;

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

  void assign(const Types::Token &token, VarValue object) {
    if (objects.contains(token)) {
      objects.insert_or_assign(token, std::move(object));
      return;
    }
    if (parentEnv != nullptr) {
      return parentEnv->assign(token, std::move(object));
    }

    throw UndefVarAccess{};
  }

  void defineOrAssign(const Types::Token &token, VarValue object) {
    if (!contains(token)) {
      objects.insert_or_assign(token, std::move(object));
      return;
    }

    assign(token, std::move(object));
  }

  VarValue get(const Types::Token &token) const {
    if (objects.contains(token)) {
      return objects.at(token);
    }
    if (parentEnv != nullptr) {
      return parentEnv->get(token);
    }

    throw UndefVarAccess{};
  }

  bool contains(const Types::Token &token) const noexcept {
    auto res = objects.contains(token);
    if (!res && parentEnv != nullptr) {
      return parentEnv->contains(token);
    }

    return res;
  }

  EnvironmentPtr getParentEnv() const noexcept { return parentEnv; }

  bool isGlobal() const noexcept { return parentEnv == nullptr; }

private:
  std::unordered_map<Token, VarValue> objects;
  EnvironmentPtr parentEnv = nullptr;
};

template <typename VarValue> class EnvironmentManager {
public:
  using EnvType = Environment<VarValue>;

  explicit EnvironmentManager(Logger &logger)
      : logger(logger),
        curEnv(std::make_shared<Environment<VarValue>>(nullptr)) {}

  template <typename F> void withNewEnviron(F &&action) {
    auto environToRestore = curEnv;
    createNewEnv();
    action();
    discardEnvsTill(environToRestore);
  }

  template <typename F>
  void withNewEnviron(std::shared_ptr<Environment<VarValue>> environ,
                      F &&action) {
    auto environToRestore = curEnv;
    curEnv = std::move(environ);
    action();
    curEnv = std::move(environToRestore);
  }

  void assign(const Types::Token &token, VarValue object) {
    try {
      curEnv->assign(token, std::move(object));
    } catch (const UndefVarAccess &e) {
      throw Errors::reportRuntimeError(logger, token,
                                       "Attempt to access an undef variable");
    }
  }

  void define(const Types::Token &token, VarValue object) {
    curEnv->defineOrAssign(token, std::move(object));
  }

  VarValue get(const Types::Token &token) const {
    try {
      return curEnv->get(token);
    } catch (const UndefVarAccess &e) {
      throw Errors::reportRuntimeError(logger, token,
                                       "Attempt to access an undef variable");
    } catch (const UninitVarAccess &e) {
      throw Errors::reportRuntimeError(logger, token,
                                       "Attempt to access an uninit variable");
    }
  }

  bool contains(const Types::Token &token) const noexcept {
    return curEnv->contains(token);
  }

private:
  void createNewEnv() {
    curEnv = std::make_shared<Environment<VarValue>>(curEnv);
  }

  void discardEnvsTill(
      const Environment<VarValue>::EnvironmentPtr &environToRestore) noexcept {
    while (!curEnv->isGlobal() && curEnv.get() != environToRestore.get()) {
      curEnv = curEnv->getParentEnv();
    }
  }

private:
  Logger &logger;
  Environment<VarValue>::EnvironmentPtr curEnv;
};

}; // namespace prsl::Types