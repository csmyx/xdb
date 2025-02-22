#include <libxdb/pipe.hpp>
#include <libxdb/process.hpp>
#include <readline.h>

namespace {
  std::unique_ptr<xdb::process> attach(int argc, char** argv) {
    pid_t pid = 0;
    if (argc == 3 && argv[1] == std::string_view("-p")) {
      pid = std::stoi(argv[2]);
      return xdb::process::attach(pid);
    } else {
      const char* prog_path = argv[1];
      return xdb::process::launch(prog_path);
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

  void print_stop_reason(const xdb::process& process, const xdb::stop_reason& reason) {
    std::cout << "Process " << process.pid() << ' ';
    switch (reason.reason) {
      case xdb::process_state::exited:
        std::cout << "exited with status " << static_cast<int>(reason.info);
        break;
      case xdb::process_state::terminated:
        std::cout << "terminated with signal " << sigabbrev_np(reason.info);
        break;
      case xdb::process_state::stopped:
        std::cout << "stopped with signal " << sigabbrev_np(reason.info);
        break;
      case xdb::process_state::running:
        break;
    }
    std::cout << std::endl;
  }

  void handle_command(std::unique_ptr<xdb::process>& process, std::string_view line) {
    auto args = split(line, ' ');
    assert(args.size() > 0);
    auto command = args[0];

    if (is_prefix(command, "continue")) {
      process->resume();
      auto reason = process->wait_on_signal();
      print_stop_reason(*process, reason);
    } else {
      std::cerr << "Unkonw command\n";
    }
  }

  void main_loop(std::unique_ptr<xdb::process>& process) {
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
        handle_command(process, line_str);
      }
    }
  }
}

int main(int argc, char** argv) {
  if (argc == 1) {
    std::cerr << "Usage: xdb <program> | xdb -p <pid>\n";
    return -1;
  }

  try {
    auto process = attach(argc, argv);
    main_loop(process);
  } catch(const xdb::error& e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }

  return 0;
}