#ifndef XDB_TYPES_HPP
#define XDB_TYPES_HPP

#include <array>
#include <cstdint>
#include <variant>

namespace xdb {
  using byte64 = std::array<std::byte, 8>;
  using byte128 = std::array<std::byte, 16>;

  using value = std::variant<
    std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t,
    std::int8_t, std::int16_t, std::int32_t, std::int64_t,
    float, double, long double,
    byte64, byte128>;
}

#endif