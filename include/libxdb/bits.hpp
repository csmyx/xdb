#ifndef XDB_BITS_HPP
#define XDB_BITS_HPP

#include <libxdb/types.hpp>
#include <cstddef>
#include <cstring>

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

  template <class From>
  byte128 as_byte128(const From& from) {
    byte128 result{};
    std::memcpy(result.data(), &from, sizeof(From));
    return result;
  }
}

#endif