#ifndef XDB_BREAKPOINT_SITE_HPP
#define XDB_BREAKPOINT_SITE_HPP

#include <cstddef>

#include "libxdb/types.hpp"

namespace xdb {
class process;
class breakpoint_site {
 public:
  breakpoint_site() = delete;
  breakpoint_site(const breakpoint_site&) = delete;
  breakpoint_site& operator=(const breakpoint_site&) = delete;

  using id_t = std::uint32_t;
  id_t id() const {
    return id_;
  }

  void enable();
  void disable();

  bool is_enabled() const {
    return is_enabled_;
  }
  virt_addr address() const {
    return addr_;
  }
  bool at_address(virt_addr addr) const {
    return addr_ == addr;
  }
  bool in_range(virt_addr start, virt_addr end) const {
    return addr_ >= start && addr_ < end;
  }

 private:
  breakpoint_site(process& proc, virt_addr addr);
  friend xdb::process;

  id_t id_;
  bool is_enabled_ = false;
  virt_addr addr_;
  xdb::process& proc_;
  std::byte data_{};
};
}  // namespace xdb

#endif