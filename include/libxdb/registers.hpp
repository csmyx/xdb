#ifndef XDB_REGISTERS_HPP
#define XDB_REGISTERS_HPP


#include <libxdb/types.hpp>
#include <libxdb/register_info.hpp>

namespace xdb {
  class process;
  class registers {
    public:
      registers() = delete;
      registers(const registers&) = delete;
      registers& operator=(const registers&) = delete;
      
      value read(const register_info& info) const;
      void write(const register_info& info, value value);

    private:
      friend process;
      registers(process& proc): proc_(&proc) {}

      user data_;
      process *proc_;
  };
}

#endif