#ifndef XDB_PROCESS_HPP
#define XDB_PROCESS_HPP

#include "libxdb/error.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <filesystem>
#include <vector>


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
      static std::unique_ptr<process> launch(std::filesystem::path path, bool trace=true);
      static std::unique_ptr<process> attach(pid_t pid);

      void resume();
      stop_reason wait_on_signal();
      pid_t pid() const { return pid_; }
      process_state state() const { return state_; }

      process() = delete;
      process(const process&) = delete;
      process& operator=(const process&) = delete;
      ~process();

    private: 
      process(pid_t pid, bool termianted_on_end, bool is_attached)
        : pid_(pid), terminated_on_end_(termianted_on_end), is_attached_(is_attached) {}

      pid_t pid_ = 0;
      process_state state_ = process_state::stopped;
      bool terminated_on_end_ = true;
      bool is_attached_ = true;
  };

}

#endif