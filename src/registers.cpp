#include <libxdb/bits.hpp>
#include <libxdb/register_info.hpp>
#include <libxdb/types.hpp>
#include <libxdb/registers.hpp>

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
