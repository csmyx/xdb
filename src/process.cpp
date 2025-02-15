#include <filesystem>
#include "libxdb/process.hpp"
#include "libxdb/error.hpp"
#include <memory>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>


std::unique_ptr<xdb::process> xdb::process::launch(std::filesystem::path path) {
  pid_t pid;
  if ((pid = fork() < 0)) {
    error::send_errno("Failed to fork");
  }

  if (pid == 0) {
    if (ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) == -1) {
      error::send_errno("Failed to PTRACE_TRACEME");
    }
    if (execlp(path.c_str(), path.c_str(), nullptr) == -1) {
      error::send_errno("Failed to execlp");
    }
  }

  std::unique_ptr<process> proc(new process(pid, /*terminate_on_end=*/true));
  proc->wait_on_signal();
  return proc;
}


std::unique_ptr<xdb::process> xdb::process::attach(pid_t pid) {
  if (pid == 0) {
    error::send("Invalid PID");
  }
  if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) < 0) {
    error::send_errno("Failed to PTRACE_ATTACH");
  }
  std::unique_ptr<process> proc(new process(pid, /*terminate_on_end=*/true));
  proc->wait_on_signal();
  return proc;
}
