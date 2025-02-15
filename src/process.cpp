#include <filesystem>
#include "libxdb/process.hpp"
#include <memory>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>


std::unique_ptr<xdb::process> xdb::process::launch(std::filesystem::path path) {
  pid_t pid;
  if ((pid = fork() < 0)) {
    std::perror("Failed to fork");
    return nullptr;
  }

  if (pid == 0) {
    if (ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) == -1) {
      std::perror("Failed to PTRACE_TRACEME");
      return nullptr;
    }
    if (execlp(path.c_str(), path.c_str(), nullptr) == -1) {
      std::perror("Failed to execlp");
      return nullptr;
    }
  }

  std::unique_ptr<process> proc(new process(pid, /*terminate_on_end=*/true));
  proc->wait_on_signal();
  return proc;
}


std::unique_ptr<xdb::process> xdb::process::attach(pid_t pid) {
  if (pid == 0) {
    std::perror("bad pid");
    return nullptr;
  }
  if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) < 0) {
    std::perror("Failed to PTRACE_ATTACH");
    return nullptr;
  }
  std::unique_ptr<process> proc(new process(pid, /*terminate_on_end=*/true));
  proc->wait_on_signal();
  return proc;
}
