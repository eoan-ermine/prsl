#pragma once

#include <string_view>

namespace prsl::Utils {

// Не все компиляторы еще поддерживают стандартный unreachable,
// Так что пусть будет такой
[[noreturn]] inline void unreachable() {
  // Uses compiler specific extensions if possible.
  // Even if no extension is used, undefined behavior is still raised by
  // an empty function body and the noreturn attribute.
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
  __assume(false);
#else // GCC, Clang
  __builtin_unreachable();
#endif
}

template <class... Ts> struct select : Ts... {
  using Ts::operator()...;
};

struct FilePos {
  std::string_view filename;
  int line;
  int col;

  static FilePos UNKNOWN() { return FilePos{"", -1, -1}; }

  static FilePos UNKNOWN(std::string_view filename) {
    return FilePos{filename, -1, -1};
  }
};

} // namespace prsl::Utils