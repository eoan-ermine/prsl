#pragma once

#include <string_view>
#include <unordered_map>

namespace prsl::Types {

template <typename T> class FunctionsManager {
public:
  // I assure you string_view comparator WON'T throw
  constexpr bool contains(std::string_view name) const noexcept {
    return functionsManager.contains(name);
  }

  constexpr const T &get(std::string_view k) const noexcept {
    return functionsManager.at(k);
  }

  constexpr void set(std::string_view k, T v) {
    functionsManager[k] = std::move(v);
  }

private:
  std::unordered_map<std::string_view, T> functionsManager;
};

} // namespace prsl::Types