#include <libxdb/pipe.hpp>
#include <libxdb/process.hpp>
#include <variant>
#include <fmt/base.h>
#include <readline.h>
#include <signal.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

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

  void print_help(const std::vector<std::string>& args) {
    if (args.size() == 1) {
      std::cerr << "Available commands:\n"
                << "\tcontinue - Resume the process\n"
                << "\tregister - Commands for operating on registers" << std::endl;
    } else if (is_prefix(args[1], "register")) {
      std::cerr << "Available commands:\n"
                << "\tread\n"
                << "\tread <register>\n"
                << "\tread all\n"
                << "\twrite <register> <value>" << std::endl;
    } else {
      std::cerr << "No help available on that" << std::endl;
    }
  }

  void handle_register_read(xdb::process& process, const std::vector<std::string>& args) {
    auto format = [](auto t) {
      if constexpr (std::is_floating_point_v<decltype(t)>) {
        return fmt::format("{}", t);
      } else if constexpr (std::is_integral_v<decltype(t)>) {
        return fmt::format("{:#0{}x}", t, sizeof(t) * 2 + 2);
      } else {
        return fmt::format("[{:#04x}]", fmt::join(t, ","));
      }
    };
    if (args.size() == 2 || (args.size() == 3 && args[2] == "all")) {
      for (auto& info: xdb::g_register_infos) {
        if (args.size() == 3 || info.type == xdb::register_type::gpr) {
          auto value =  process.get_registers().read(info);
          fmt::print("{}:\t{}\n", info.name, std::visit(format, value));
        }
      }
    } else if (args.size() == 3) {
      try {
        auto& info = xdb::register_info_by_name(args[2]);
        auto value = process.get_registers().read(info);
        fmt::print("{}:\t{}\n", info.name, std::visit(format, value));
      } catch(xdb::error& e) {
        fmt::print(stderr, "{}: {}\n", e.what(), args[2]);
      }
    } else {
      std::cerr << "Invalid arguments for register" << std::endl;
      print_help({ "help", "register" });
    }
  }

    void handle_register_write(xdb::process & process, const std::vector<std::string>& args) {}

    void handle_register_command(xdb::process & process, const std::vector<std::string>& args) {
      if (args.size() < 2) {
        print_help({"help", "register"});
        return;
      }
      if (is_prefix(args[1], "read")) {
        handle_register_read(process, args);
      } else if (is_prefix(args[1], "write")) {
        handle_register_write(process, args);
      } else {
        print_help({"help", "register"});
      }
    }

    void handle_command(std::unique_ptr<xdb::process> & process, std::string_view line) {
      auto args = split(line, ' ');
      assert(args.size() > 0);
      auto command = args[0];

      if (is_prefix(command, "continue")) {
        process->resume();
        auto reason = process->wait_on_signal();
        print_stop_reason(*process, reason);
      } else if (is_prefix(command, "register")) {
        handle_register_command(*process, args);
      } else if (is_prefix(command, "help")) {
        print_help(args);
      } else if (is_prefix(command, "quit")) {
        assert(kill(process->pid(), SIGTERM) == 0);
        process->wait_on_signal();
        std::cerr << static_cast<int>(process->state()) << std::endl;
        exit(0);
      } else {
        std::cerr << "Unkonw command\n";
      }
    }

    void main_loop(std::unique_ptr<xdb::process> & process) {
      char* line = nullptr;
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