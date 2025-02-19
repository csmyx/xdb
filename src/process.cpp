#include <filesystem>
#include "libxdb/process.hpp"
#include "libxdb/error.hpp"
#include "libxdb/pipe.hpp"
#include <memory>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>

namespace xdb {
  // exit the process after sending error message to channel
  [[noreturn]]
  void exit_with_error(xdb::pipe& channel, std::string const& prefix) {
    auto message = prefix + ": " + std::strerror(errno);
    channel.write(reinterpret_cast<std::byte*>(message.data()), message.size());
    exit(-1);
  }
}

xdb::stop_reason::stop_reason(int wait_status) {
  if (WIFSTOPPED(wait_status)) {
    reason = process_state::stopped;
    info = WSTOPSIG(wait_status);
  } else if (WIFEXITED(wait_status)) {
    reason = process_state::exited;
    info = WEXITSTATUS(wait_status);
  } else if (WIFSIGNALED(wait_status)) {
    reason = process_state::terminated;
    info = WTERMSIG(wait_status);
  } else {
    reason = process_state::running;
    info = 0;
  }
}

/// trace: whether to trace the child process (set to false for testing to create a untraced process)
std::unique_ptr<xdb::process> xdb::process::launch(std::filesystem::path path, bool trace) {
  xdb::pipe channel(true); // pass errors from child to parent
  pid_t pid;
  if ((pid = fork()) < 0) {
    error::send_errno("Failed to fork");
  }

  if (pid == 0) {
    channel.close_read();
    if (trace && ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) < 0) {
      exit_with_error(channel, "Failed to PTRACE_TRACEME");
    }
    if (execlp(path.c_str(), path.c_str(), nullptr) < 0) {
      exit_with_error(channel, "Failed to execlp");
    }
  }

  // get error message from child
  auto err_msg = channel.finish_read();
  if (!err_msg.empty()) {
    auto from = reinterpret_cast<char*>(err_msg.data());
    error::send(std::string(from, from + err_msg.size()));
  }

  std::unique_ptr<process> proc(new process(pid, /*terminate_on_end=*/true, trace));
  if (trace) {
    proc->wait_on_signal();
  }
  return proc;
}


std::unique_ptr<xdb::process> xdb::process::attach(pid_t pid) {
  if (pid <= 0) {
    error::send("Invalid PID");
  }
  if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) < 0) {
    error::send_errno("Failed to PTRACE_ATTACH");
  }
  std::unique_ptr<process> proc(new process(pid, /*terminate_on_end=*/false, true));
  proc->wait_on_signal();
  return proc;
}

xdb::process::~process() {
  if (pid_ != 0) {
    int status;

    // stop it before detaching
    if (is_attached_) {
      if (state_ == process_state::running) {
        kill(pid_, SIGSTOP);
        waitpid(pid_, &status, 0);
      }
      // detach and let it continue
      ptrace(PTRACE_DETACH, pid_, nullptr, nullptr);
      kill(pid_, SIGCONT);
    }

    // terminate it if needed
    if (terminated_on_end_) {
      kill(pid_, SIGKILL);
      waitpid(pid_, &status, 0);
    }
  }
}

void xdb::process::resume() {
  if (ptrace(PTRACE_CONT, pid_, nullptr, nullptr) < 0) {
    error::send_errno("Failed to PTRACE_CONT");
  }
  state_ = process_state::running;
}

xdb::stop_reason xdb::process::wait_on_signal() {
  int wait_status;
  int options = 0;
  if (waitpid(pid_, &wait_status, options) < 0) {
    error::send_errno("Failed to waitpid");
  }
  stop_reason reason(wait_status);
  state_ = reason.reason;
  return reason;
}

