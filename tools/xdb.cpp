#include <cassert>
#include <cstdio>
#include "libxdb/libxdb.hpp"
#include <iostream>
#include <unistd.h>
#include <string_view>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string>
#include <vector>
#include <editline/readline.h>
#include <sstream>

namespace {
  pid_t attach(int argc, char** argv);
  void handle_command(pid_t pid, std::string_view line);
  std::vector<std::string> split(std::string_view str, char delimiter);
  bool is_prefix(std::string_view str, std::string_view of);
  void resume(pid_t pid);
  void wait_on_signal(pid_t pid);
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

  void handle_command(pid_t pid, std::string_view line) {
    auto args = split(line, ' ');
    assert(args.size() > 0);
    auto command = args[0];

    if (is_prefix(command, "continue")) {
      resume(pid);
      wait_on_signal(pid);
    } else {
      std::cerr << "Unkonw command\n";
    }
  }

  void resume(pid_t pid) {
    if (ptrace(PTRACE_CONT, pid, nullptr, nullptr) < 0) {
      std::cerr << "Couldn't conintue\n";
      std::exit(-1);
    }
  }

  void wait_on_signal(pid_t pid) {
    int wait_status;
    int options = 0;
    if (waitpid(pid, &wait_status, options) < 0) {
      std::perror("Waitpid failed");
      std::exit(-1);
    }
  }

  std::vector<std::string> split(std::string_view str, char delimiter) {
    std::vector<std::string> out{};
    std::stringstream ss {std::string{str}};
    std::string item;
    while (std::getline(ss, item, delimiter)) {
      out.push_back(item);
    }
    return out;
  }

  bool is_prefix(std::string_view str, std::string_view of) {
    if (str.size() > of.size()) return false;
    return std::equal(str.begin(), str.end(), of.begin());
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

  std::cout << "****test****" << std::endl;

  char *line = nullptr;
  while ((line = readline("xdb> ")) != nullptr) {
    std::string line_str;

    // if the input is empty, use the last command
    if (line == std::string_view("")) {
      free(line);
      if (history_length > 0) {
        line_str = history_list()[history_length - 1]->line;
      }
    } else {
      line_str = line;
      add_history(line);
      free(line);
    }
      
    if (!line_str.empty()) {
      handle_command(pid, line_str);
    }
  }
  
}