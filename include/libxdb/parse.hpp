#ifndef XDB_PARSE_HPP
#define XDB_PARSE_HPP

#include <array>
#include <cctype>
#include <charconv>
#include <cstddef>
#include <optional>
#include <string_view>
#include <vector>
#include "libxdb/types.hpp"

namespace xdb {
template<class T>
std::optional<T> to_integer(std::string_view str, int base) {
  T result{};
  auto ret = std::from_chars(str.begin(), str.end(), result, base);
  if (ret.ptr != str.end()) {
    return std::nullopt;
  }
  return result;
}

template<class T>
std::optional<T> to_integer(std::string_view str) {
  auto beg = str.begin();
  int base = 10;
  std::size_t len = str.size();
  if (len >= 2 && str[0] == '0' && str[1] == 'x') {
    base = 16;
    beg += 2;
    len -= 2;
  }
  return to_integer<T>({beg, len}, base);
}

template <>
inline std::optional<std::byte> to_integer(std::string_view str, int base) {
  auto uint8 = to_integer<std::uint8_t>(str, base);
  if (uint8) return static_cast<std::byte>(*uint8);
  return std::nullopt;
}

template<class T>
std::optional<T> to_float(std::string_view str) {
  auto begin = str.begin();
  auto end = str.end();
  T result{};
  auto ret = std::from_chars(begin, end, result);
  if (ret.ptr != end) {
    return std::nullopt;
  }
  return result;
}

template<std::size_t N>
std::optional<std::array<std::byte, N>> to_vector(std::string_view str) {
  std::array<std::byte, N> result;
  auto beg = str.begin();
  auto end = str.end();

  // Remove leading and trailing whitespace
  while (beg != end && std::isspace(*beg)) ++beg;
  while (beg != end && std::isspace(*(end - 1))) --end;

  // Check for the opening and closing brackets
  if (beg == end || *beg != '[' || *(end - 1) != ']') {
    return std::nullopt;
  }
  ++beg; // Skip the opening bracket
  --end;   // Skip the closing bracket

  std::size_t idx = 0;
  auto is_delim = [](char c) { return std::isspace(c) || c == ','; };
  while (beg != end) {
    // Skip any whitespace
    while (beg != end && is_delim(*beg)) ++beg;

    std::size_t len = 1;
    while (beg+len != end && !is_delim(*(beg+len))) {
      ++len;
    }

    int base = 10;
    if (len >= 2 && *beg == '0' && *(beg + 1) == 'x') {
      base = 16;
      beg += 2;
      len -= 2;
    }
    auto value = to_integer<std::byte>({beg, len}, base);
    if (!value) {
      return std::nullopt;
    }
    result[idx++] = value.value();
    beg += len;

    if (idx >= N) {
      break;
    }
  }

  // Skip any trailing whitespace
  while (beg != end && is_delim(*beg)) ++beg;

  if (beg != end || idx != N) {
    return std::nullopt;
  }
  return result;
}

}

#endif