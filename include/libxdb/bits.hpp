#ifndef XDB_BITS_HPP
#define XDB_BITS_HPP

#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <libxdb/types.hpp>
#include <cstddef>
#include <cstring>
#include <string_view>
#include <vector>

namespace xdb {
  template<class To>
  To from_bytes(const std::byte* byte) {
    To result;
    std::memcpy(&result, byte, sizeof(To));
    return result;
  }

  template<class From>
  std::byte* as_bytes(From& from) {
    return reinterpret_cast<std::byte*>(&from);
  }

  template <class From>
  const std::byte* as_bytes(const From& from) {
    return reinterpret_cast<const std::byte*>(&from);
  }

  template <class From>
  byte64 as_byte64(const From& from) {
    byte64 result{};
    std::memcpy(result.data(), &from, sizeof(From));
    return result;
  }
  inline byte64 as_byte64(std::initializer_list<std::uint8_t> from) {
    byte64 result{};
    std::transform(from.begin(), from.end(), result.begin(),
    [](uint8_t value) { return std::byte{value}; });
    return result;
  }

  template <class From>
  byte128 as_byte128(const From& from) {
    byte128 result{};
    std::memcpy(result.data(), &from, sizeof(From));
    return result;
  }
  inline byte128 as_byte128(std::initializer_list<std::uint8_t> from) {
    byte128 result{};
    std::transform(from.begin(), from.end(), result.begin(),
    [](uint8_t value) { return std::byte{value}; });
    return result;
  }
  
  inline std::string_view to_string_view(const std::byte* data, std::size_t size) {
    return { reinterpret_cast<const char*>(data), size };
  }
  
  inline std::string_view to_string_view(const std::vector<std::byte>& data) {
    return to_string_view(data.data(), data.size());
  }
}

#endif