#ifndef XDB_PROCESS_HPP
#define XDB_PROCESS_HPP

#include <memory>
#include <filesystem>

namespace xdb {
  enum class process_state {
    stopped,
    running,
    exited,
    terminated
  };

  class process {
    public:
      static std::unique_ptr<process> launch(std::filesystem::path path);
      static std::unique_ptr<process> attach(pid_t pid);

      void resume();
      void wait_on_signal();
      pid_t pid() const { return pid_; }
      process_state state() const { return state_; }

      process() = delete;
      process(const process&) = delete;
      process& operator=(const process&) = delete;
      ~process();

    private: 
      process(pid_t pid, bool termiante_on_end)
        : pid_(pid), terminated_on_end_(termiante_on_end) {}

      pid_t pid_ = 0;
      process_state state_ = process_state::stopped;
      bool terminated_on_end_ = true;
  };

}

#endif