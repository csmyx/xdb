#include <iostream>
#include <libxdb/bits.hpp>
#include <libxdb/register_info.hpp>
#include <libxdb/types.hpp>
#include <libxdb/process.hpp>
#include <type_traits>

namespace xdb {
  // widen the value to the register size
  template<class T>
  xdb::byte128 widen(const xdb::register_info& info, T t) {
    if constexpr (std::is_floating_point_v<T>) {
      if (info.format == register_format::double_float) {
        return as_byte128(static_cast<double>(t));
      }
      if (info.format == register_format::long_double) {
        return as_byte128(static_cast<long double>(t));
      }
    } else if constexpr (std::is_signed_v<T>) {
      if (info.format == register_format::uint) {
        switch (info.size) {
          case 2: return as_byte128(static_cast<std::int16_t>(t));
          case 4: return as_byte128(static_cast<std::int32_t>(t));
          case 8: return as_byte128(static_cast<std::int64_t>(t));
        }
      }
    }
    return as_byte128(t);
  }
}

xdb::value xdb::registers::read(const register_info& info) const {
  auto bytes = as_bytes(data_);
  if (info.format == register_format::uint) {
    switch(info.size) {
      case 1:
        return from_bytes<std::uint8_t>(bytes + info.offset);
      case 2:
        return from_bytes<std::uint16_t>(bytes + info.offset);
      case 4:
        return from_bytes<std::uint32_t>(bytes + info.offset);
      case 8:
        return from_bytes<std::uint64_t>(bytes + info.offset);
      default:
        error::send("Invalid register size");
    }
  } else if (info.format == register_format::double_float) {
    return from_bytes<double>(bytes + info.offset);
  } else if (info.format == register_format::long_double) {
    return from_bytes<long double>(bytes + info.offset);
  } else if (info.format == register_format::vector) {
    if (info.size == 16) {
      return from_bytes<byte64>(bytes + info.offset);
    } else if (info.size == 8) {
      return as_byte64(bytes + info.offset);
    } else {
      error::send("Invalid register size");
    }
  } else {
    error::send("Invalid register format");
  }
}

void xdb::registers::write(const xdb::register_info& info, xdb::value value) {
  auto bytes = as_bytes(data_);
  std::visit([&](auto& v){
    if (sizeof(v) <= info.size) {
      auto aligned_v = widen(info, v);
      auto ptr = as_bytes(aligned_v);
      // I think we can also use std::memcopy rather than std::copy here
      std::copy(ptr, ptr + info.size, bytes + info.offset);
    } else {
      std::cerr << "xdb::register::write called with "
      "mismatched register and value sizes";
      std::terminate();
    }
  }, value);

  // use PTRACE_SETFPREGS to write floating point registers
  if (info.type == register_type::fpr) {
    proc_->write_fprs(data_.i387);
  } else {
    // align the offset to 8 bytes
    auto aligned_offset = info.offset & ~0b111;
    proc_->write_user_area(aligned_offset, 
      from_bytes<std::uint64_t>(bytes + aligned_offset));
  }
}
