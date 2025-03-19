#include <libxdb/breakpoint_site.hpp>
#include <libxdb/process.hpp>

namespace xdb {
  auto get_next_id() {
    static breakpoint_site::id_t id = 0;
    return ++id;
  }

  void breakpoint_site::enable() {

  }

  void breakpoint_site::disable() {

  }

  breakpoint_site::breakpoint_site(process& proc, virt_addr addr) : id_(get_next_id()), addr_(addr), proc_(proc) {}
}
void enable();
void disable();
