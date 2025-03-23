#ifndef XDB_PROCESS_HPP
#define XDB_PROCESS_HPP

#include <libxdb/registers.hpp>
#include <cstdint>
#include <libxdb/error.hpp>
#include <cassert>
#include <iostream>
#include <memory>
#include <filesystem>
#include <optional>
#include <vector>
#include "libxdb/stoppoint_manager.hpp"
#include "libxdb/types.hpp"
#include <libxdb/breakpoint_site.hpp>

namespace xdb {
  enum class process_state {
    stopped,
    running,
    exited,
    terminated
  };

  struct stop_reason {
    stop_reason(int wait_status);
    process_state reason;
    std::uint8_t info;
  };

  class process {
    public:
      // debug entry of launching a new process
      static std::unique_ptr<process> launch(std::filesystem::path path, bool trace=true, std::optional<int> stdout_replacement=std::nullopt);
      // debug entry of attaching to an existing process
      static std::unique_ptr<process> attach(pid_t pid);

      void resume();
      stop_reason wait_on_signal();
      pid_t pid() const { return pid_; }
      process_state state() const { return state_; }

      process() = delete;
      process(const process&) = delete;
      process& operator=(const process&) = delete;
      ~process();

      registers& get_registers() { return *regs_; } 
      const registers& get_registers() const { return *regs_; }
      // write regisers
      void write_user_area(std::size_t offset, std::uint64_t data);
      void write_fprs(const user_fpregs_struct& fprs);
      void write_gprs(const user_regs_struct& gprs);

      xdb::virt_addr get_pc() const {
        return xdb::virt_addr(regs_->read_by_id<std::uint64_t>(register_id::rip));
      } 


      // breakpoint management
      breakpoint_site& create_breakpoint_site(virt_addr addr);


    private: 
      process(pid_t pid, bool termianted_on_end, bool is_attached)
        : pid_(pid), terminated_on_end_(termianted_on_end),
        is_attached_(is_attached), regs_(new registers(*this)) {}
      
      // read regisers
      void read_all_registers();

      pid_t pid_ = 0;
      process_state state_ = process_state::stopped;
      bool terminated_on_end_ = true;
      bool is_attached_ = true;
      std::unique_ptr<registers> regs_;
      stoppoint_manager<breakpoint_site> breakpoint_sites_;
  };

}

#endif