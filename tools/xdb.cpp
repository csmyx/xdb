#include <cstdio>
#include "libxdb/libxdb.hpp"
#include <iostream>
#include <unistd.h>
#include <string_view>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string>

namespace {
  pid_t attach(int argc, char** argv);
  void handle_command(pid_t pid, std::string_view line);
}

namespace {
  pid_t attach(int argc, char** argv) {
    pid_t pid = 0;
    if (argc == 3 && argv[1] == std::string_view("-p")) {
      pid = std::stoi(argv[2]);
      if (pid <= 0) {
        std::cerr << "Invalid pid" << std::endl;
        return -1;
      }
      if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) == -1) {
        std::perror("Failed to PTRACE_ATTACH");
        return -1;
      }
    } else {
      const char* prog_path = argv[1];
      if ((pid = fork()) < 0) {
        std::perror("Failed to fork");
        return -1;
      }
      if (pid == 0) {
        if (ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) == -1) {
          std::perror("Failed to PTRACE_TRACEME");
          return -1;
        }
        // due to the ptrace(PTRACE_TRACEME) call before exec, 
        // this child process will keep stopping until a state change caused by a signal
        if (execlp(prog_path, prog_path, nullptr) == -1) {
          std::perror("Failed to execlp");
          return -1;
        } 
      }
    }
    return pid;
  }
}

int main(int argc, char** argv) {
  if (argc == 1) {
    std::cerr << "No argments given" << std::endl;
    return -1;
  }
  pid_t pid = attach(argc, argv);
  
  int wait_status;
  int options = 0;
  // wait for child process
  if (waitpid(pid, &wait_status, options) == -1) {
    std::perror("Failed to waitpid");
    return -1;
  }

  char *line = nullptr;
}