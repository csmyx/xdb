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

  // Stand for virtual address
  class virt_addr {
   public:
    virt_addr() = default;
    explicit virt_addr(std::uint64_t addr) : addr_(addr) {}
    std::uint64_t addr() const { return addr_; }

    virt_addr operator+(std::uint64_t offset) const {
      return virt_addr(addr_ + offset);
    }
    virt_addr operator-(std::uint64_t offset) const {
      return virt_addr(addr_ - offset);
    }
    virt_addr& operator+=(std::uint64_t offset) {
      addr_ += offset;
      return *this;
    }
    virt_addr& operator-=(std::uint64_t offset) {
      addr_ -= offset;
      return *this;
    }
    bool operator==(const virt_addr& rhs) const {
      return addr_ == rhs.addr_;
    }
    bool operator!=(const virt_addr& rhs) const {
      return addr_ != rhs.addr_;
    }
    bool operator<(const virt_addr& rhs) const {
      return addr_ < rhs.addr_;
    }
    bool operator>(const virt_addr& rhs) const {
      return addr_ > rhs.addr_;
    }
    bool operator<=(const virt_addr& rhs) const {
      return addr_ <= rhs.addr_;
    }
    bool operator>=(const virt_addr& rhs) const {
      return addr_ >= rhs.addr_;
    }

   private:
    std::uint64_t addr_ = 0;
  };

  }  // namespace xdb

#endif