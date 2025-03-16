#include <fmt/base.h>
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <cstdint>
#include <libxdb/process.hpp>
#include <libxdb/bits.hpp>
#include <libxdb/pipe.hpp>
#include <csignal>
#include <fstream>
#include "libxdb/types.hpp"
#include <fmt/format.h>
#include <fmt/ranges.h>

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
  auto target =  process::launch("targets/run_endlessly", false);
  auto proc = process::attach(target->pid());
  REQUIRE(get_process_status(target->pid()) == 't');
}
TEST_CASE("process::resume success", "[process]") {
  {
    auto proc =  process::launch("targets/run_endlessly");
    REQUIRE(get_process_status(proc->pid()) == 't');
    proc->resume();
    REQUIRE(get_process_status(proc->pid()) == 'R');
  }
  {
    auto target =  process::launch("targets/run_endlessly", false);
    auto proc = process::attach(target->pid());
    REQUIRE(get_process_status(proc->pid()) == 't');
    proc->resume();
    REQUIRE(get_process_status(proc->pid()) == 'R');
  }
}
TEST_CASE("process::resume already terminated", "[process]") {
  auto proc = process::launch("targets/end_immediately");
  std::cout << get_process_status(proc->pid()) << std::endl;
  proc->resume();
  std::cout << get_process_status(proc->pid()) << std::endl;
  proc->wait_on_signal();
  // proc has already terminated
  REQUIRE_THROWS_AS(proc->resume(), error);
}
TEST_CASE("registers: writing issue1", "[registers]") {
  auto target =  process::launch("targets/run_endlessly", false);
  auto proc = process::attach(target->pid());
  auto& reg = proc->get_registers();
  reg.write(register_info_by_name("ah"), std::uint8_t{0x12});
}
TEST_CASE("registers: writing issue2", "[registers]") {
  auto target =  process::launch("targets/run_endlessly", false);
  auto proc = process::attach(target->pid());
  auto& reg = proc->get_registers();
  // test writing FPR registers
  reg.write(register_info_by_name("fcw"), std::uint16_t{0x0102});
}
TEST_CASE("registers: writing issue3", "[registers]") {
  auto target =  process::launch("targets/run_endlessly", false);
  auto proc = process::attach(target->pid());
  auto& reg = proc->get_registers();
  
  reg.write(register_info_by_name("rax"), std::uint16_t{0x02});
}

void test_reg_write_helper(xdb::process& proc, xdb::register_id id, xdb::value value) {
  assert(proc.state() == xdb::process_state::stopped);
  auto& regs = proc.get_registers();
  regs.write_by_id(id, value);

  proc.resume();
  proc.wait_on_signal();
}
TEST_CASE("write rigister wordks", "[register]") {
  bool close_on_exec = false;
  xdb::pipe channel(close_on_exec);
  auto proc= xdb::process::launch("targets/reg_write", true, channel.get_write_fd());
  channel.close_write();
  proc->resume();
  proc->wait_on_signal();

  {
    test_reg_write_helper(*proc, register_id::rsi, 0x01020304);
    auto output = channel.read();
    REQUIRE(to_string_view(output) == "0x01020304");
  }

  {
    test_reg_write_helper(*proc, register_id::mm0, 0x0102030405060708);
    auto output = channel.read();
    REQUIRE(to_string_view(output) == "0x0102030405060708");
  }

  {
    test_reg_write_helper(*proc, register_id::xmm0, 3.14);
    auto output = channel.read();
    REQUIRE(to_string_view(output) == "3.14");
  }
  
  {
    auto& regs = proc->get_registers();
    regs.write_by_id(register_id::st0, 42.24l);
    regs.write_by_id(register_id::fsw, std::uint16_t{0b0011100000000000});
    regs.write_by_id(register_id::ftw, std::uint16_t{0b0011111111111111});
    proc->resume();
    proc->wait_on_signal();
    auto output = channel.read();
    REQUIRE(to_string_view(output) == "42.24");
  }
}

TEST_CASE("read rigister wordks", "[register]") {
  auto proc= xdb::process::launch("targets/reg_read");
  auto& regs = proc->get_registers();
  proc->resume();
  proc->wait_on_signal();

  {
    auto value = regs.read_by_id<std::uint64_t>(register_id::r13);
    REQUIRE(value == 0xcafecafe);
    proc->resume();
    proc->wait_on_signal();
  }

  {
    auto value = regs.read_by_id<std::uint8_t>(register_id::r13b);
    REQUIRE(value == 42);
    proc->resume();
    proc->wait_on_signal();
  }

  {
    auto value = regs.read_by_id<xdb::byte64>(register_id::mm0);
    REQUIRE(value == xdb::as_byte64(0x01020304ULL));
    proc->resume();
    proc->wait_on_signal();
  }

  {
    auto value = regs.read_by_id<xdb::byte128>(register_id::xmm0);
    REQUIRE(value == xdb::as_byte128(64.125));
    proc->resume();
    proc->wait_on_signal();
  }

  {
    auto value = regs.read_by_id<long double>(register_id::st0);
    REQUIRE(value == 64.125L);
    proc->resume();
    proc->wait_on_signal();
  }
}

#include <libxdb/parse.hpp>
TEST_CASE("parse register value", "[parser]") {
  {
    std::vector<std::pair<std::string, std::uint32_t>> list = {
      {"0x12", 18},
      {"12", 12},
    };
    for (auto& [raw, expect]: list) {
      auto v = to_integer<std::uint32_t>(raw);
      REQUIRE(v.has_value());
      // fmt::println("{}, {}", v.value(), expect);
      REQUIRE(v.value() == expect);
    }
  }

  {
    std::vector<std::pair<std::string, xdb::byte64>> list = {
      {"[1,2,3,4,5,6,7,8]", as_byte64({1,2,3,4,5,6,7,8})},
    };
    for (auto& [raw, expect]: list) {
      auto v = to_vector<8>(raw);
      REQUIRE(v.has_value());
      // fmt::println("{}, {}", v.value(), expect);
      REQUIRE(v.value() == expect);
    }
  }

  {
    std::vector<std::pair<std::string, xdb::byte128>> list = {
      {"[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]", 
        as_byte128({1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16})
      }
    };
    for (auto& [raw, expect]: list) {
      auto v = to_vector<16>(raw);
      REQUIRE(v.has_value());
      // fmt::println("{}, {}", v.value(), expect);
      REQUIRE(v.value() == expect);
    }
  }

  {
    std::vector<std::tuple<std::string, xdb::byte128, bool>> list = {
      {"  [  1, 2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,]  ", 
        as_byte128({1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}),
        true
      },
      {"  [  1, 2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,]  ", 
        as_byte128({1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}),
        false
      },
      {"  [  1, 2,3,4,5,6,7,8,9,10,11,12,13,14,15,,,]  ", 
        as_byte128({1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}),
        false
      }
    };
    for (auto& [raw, expect, flag]: list) {
      auto v = to_vector<16>(raw);
      if (flag) {
        REQUIRE(v.has_value());
        REQUIRE(v.value() == expect);
      } else {
        REQUIRE(!v.has_value());
      }
    }
  }
}
