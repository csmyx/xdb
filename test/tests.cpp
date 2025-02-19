#include <catch2/catch_test_macros.hpp>
#include <libxdb/process.hpp>
#include <csignal>
#include <fstream>

using namespace xdb;
namespace {
  bool process_exists(pid_t pid) {
    auto ret = kill(pid, 0);
    return ret != -1 && errno != ESRCH; 
  }

  char get_process_status(pid_t pid) {
    std::ifstream stat("/proc/" + std::to_string(pid) + "/stat");
    std::string data;
    std::getline(stat, data);
    auto index_of_last_parenthesis = data.rfind(')');
    auto index_of_status_indicator = index_of_last_parenthesis + 2;
    return data[index_of_status_indicator];
  }
}

TEST_CASE("process::launch success", "[process]") {
  auto proc = process::launch("yes");
  REQUIRE(process_exists(proc->pid()));
}
TEST_CASE("process::launch no such program", "[process]") {
  REQUIRE_THROWS_AS(process::launch("you_do_not_have_to_be_good"), error);
}
TEST_CASE("process::attach success", "[process]") {
  auto target =  process::launch("build/test/targets/run_endlessly", false);
  auto proc = process::attach(target->pid());
  REQUIRE(get_process_status(target->pid()) == 't');
}
TEST_CASE("process::resume success", "[process]") {
  {
    auto proc =  process::launch("build/test/targets/run_endlessly");
    REQUIRE(get_process_status(proc->pid()) == 't');
    proc->resume();
    REQUIRE(get_process_status(proc->pid()) == 'R');
  }
  {
    auto target =  process::launch("build/test/targets/run_endlessly", false);
    auto proc = process::attach(target->pid());
    REQUIRE(get_process_status(proc->pid()) == 't');
    proc->resume();
    REQUIRE(get_process_status(proc->pid()) == 'R');
  }
}
TEST_CASE("process::resume already terminated", "[process]") {
  auto proc = process::launch("build/test/targets/end_immediately");
  std::cout << get_process_status(proc->pid()) << std::endl;
  proc->resume();
  std::cout << get_process_status(proc->pid()) << std::endl;
  proc->wait_on_signal();
  // proc has already terminated
  REQUIRE_THROWS_AS(proc->resume(), error);
}
