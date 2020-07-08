#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "cbus.hpp"
#include "doctest.h"
#include <string>

struct virtual_bus {
  void register_handler(std::function<void(const std::string&)> feed) { virtual_bus::feed = feed; }
  std::function<void(const std::string&)> feed;
};

TEST_CASE("test close on garbage") {
  uint64_t time = 0;
  cbus::config cfg;
  cfg.now = [&time] { return time; };
  cfg.close_on_timeout = true;
  cfg.address = 0x42;
  std::shared_ptr<virtual_bus> vbus = std::make_shared<virtual_bus>();
  cbus::bus<virtual_bus> b(vbus, cfg, [](const cbus::single_packet&) {});
  CHECK(b.open());
  vbus->feed("a");
  CHECK(b.open());
  time += 1005;
  CHECK(b.open());
}

TEST_CASE("test simple receive of tcp packet") {
  uint64_t time = 0;
  uint_least32_t cnt = 0;
  cbus::config cfg;
  cfg.now = [&time] { return time; };
  cfg.close_on_timeout = true;
  cfg.use_tcp_format = true;
  cfg.is_master = false;
  cfg.address = 0x42;
  std::shared_ptr<virtual_bus> vbus = std::make_shared<virtual_bus>();
  cbus::bus<virtual_bus> b(vbus, cfg, [&cnt](const cbus::single_packet& pkg) {
    cnt++;
    CHECK(std::holds_alternative<cbus::read_coils_request>(pkg));
    CHECK(std::get<cbus::read_coils_request>(pkg).first_coil == 0x100);
    CHECK(std::get<cbus::read_coils_request>(pkg).coil_count == 1);
  });
  CHECK(b.open());
  CHECK(cnt == 0);
  std::string data("\x00\x00\x00\x00\x00\x06\x42\x01\x01\x00\x00\x01", 12);
  vbus->feed(data);
  CHECK(b.open());
  CHECK(cnt == 1);
}

TEST_CASE("test simple receive of rtu packet (wikipedia)") {
  uint64_t time = 0;
  uint_least32_t cnt = 0;
  cbus::config cfg;
  cfg.now = [&time] { return time; };
  cfg.close_on_timeout = true;
  cfg.use_tcp_format = false;
  cfg.is_master = true;
  cfg.address = 0;
  std::shared_ptr<virtual_bus> vbus = std::make_shared<virtual_bus>();
  cbus::bus<virtual_bus> b(vbus, cfg, [&cnt](const cbus::single_packet& pkg) {
    cnt++;
    CHECK(std::holds_alternative<cbus::read_input_registers_response>(pkg));
    CHECK(std::get<cbus::read_input_registers_response>(pkg).register_data.size() == 2);
    CHECK(((uint8_t)std::get<cbus::read_input_registers_response>(pkg).register_data.at(0)) == ((uint8_t)0xff));
    CHECK(((uint8_t)std::get<cbus::read_input_registers_response>(pkg).register_data.at(1)) == ((uint8_t)0xff));
  });
  CHECK(b.open());
  CHECK(cnt == 0);
  std::string data("\x01\x04\x02\xff\xff\xb8\x80", 7);
  vbus->feed(data);
  CHECK(b.open());
  CHECK(cnt == 1);
}

/*TEST_CASE("test simple receive of rtu packet") {
  uint64_t time = 0;
  uint_least32_t cnt = 0;
  cbus::config cfg;
  cfg.now = [&time] { return time; };
  cfg.close_on_timeout = true;
  cfg.use_tcp_format = false;
  cfg.is_master = true;
  cfg.address = 0x42;
  std::shared_ptr<virtual_bus> vbus = std::make_shared<virtual_bus>();
  cbus::bus<virtual_bus> b(vbus, cfg, [&cnt](const cbus::single_packet& pkg) {
    cnt++;
    CHECK(std::holds_alternative<cbus::read_coils_response>(pkg));
    CHECK(std::get<cbus::read_coils_response>(pkg).coil_data.size() == 1);
    CHECK(std::get<cbus::read_coils_response>(pkg).coil_data.at(0) == 0x34);
  });
  CHECK(b.open());
  CHECK(cnt == 0);
  std::string data("\x42\x01\x01\x34\x38\x42", 6);
  vbus->feed(data);
  CHECK(b.open());
  CHECK(cnt == 1);
}*/

TEST_CASE("test simple ignore of tcp packet for other addr") {
  uint64_t time = 0;
  uint_least32_t cnt = 0;
  cbus::config cfg;
  cfg.now = [&time] { return time; };
  cfg.close_on_timeout = true;
  cfg.use_tcp_format = true;
  cfg.is_master = false;
  cfg.address = 0x42;
  std::shared_ptr<virtual_bus> vbus = std::make_shared<virtual_bus>();
  cbus::bus<virtual_bus> b(vbus, cfg, [&cnt](const cbus::single_packet& pkg) {
    cnt++;
    CHECK(std::holds_alternative<cbus::read_coils_request>(pkg));
    CHECK(std::get<cbus::read_coils_request>(pkg).first_coil == 0x100);
    CHECK(std::get<cbus::read_coils_request>(pkg).coil_count == 1);
  });
  CHECK(b.open());
  CHECK(cnt == 0);
  std::string data("\x00\x00\x00\x00\x00\x06\x43\x01\x01\x00\x00\x01", 12);
  vbus->feed(data);
  CHECK(b.open());
  CHECK(cnt == 0);
}

TEST_CASE("test simple receive of slow multiple tcp packets") {
  uint64_t time = 0;
  uint_least32_t cnt = 0;
  cbus::config cfg;
  cfg.now = [&time] { return time; };
  cfg.close_on_timeout = true;
  cfg.use_tcp_format = true;
  cfg.is_master = false;
  cfg.address = 0x42;
  std::shared_ptr<virtual_bus> vbus = std::make_shared<virtual_bus>();
  cbus::bus<virtual_bus> b(vbus, cfg, [&cnt](const cbus::single_packet& pkg) {
    cnt++;
    CHECK(std::holds_alternative<cbus::read_coils_request>(pkg));
    CHECK(std::get<cbus::read_coils_request>(pkg).first_coil == 0x100);
    CHECK(std::get<cbus::read_coils_request>(pkg).coil_count == 1);
  });
  CHECK(b.open());
  CHECK(cnt == 0);
  std::string data("\x00\x00\x00\x00\x00\x06\x42\x01\x01\x00\x00\x01", 12);
  data = data + data + data + data + data + data + data + data;
  for (uint_least32_t i = 0; i < data.size(); i += 13) {
    vbus->feed(data.substr(i, 13));
    // time += 500;
  }
  CHECK(b.open());
  CHECK(cnt == 8);
}

TEST_CASE("test invalid protocol id") {
  uint64_t time = 0;
  cbus::config cfg;
  cfg.now = [&time] { return time; };
  cfg.close_on_timeout = true;
  cfg.use_tcp_format = true;
  cfg.address = 0x42;
  std::shared_ptr<virtual_bus> vbus = std::make_shared<virtual_bus>();
  cbus::bus<virtual_bus> b(vbus, cfg, [](const cbus::single_packet&) {});
  CHECK(b.open());
  std::string data("\x00\x00\x00\x01\x00\x06\x00\x01\x01\x00\x00\x01", 5);
  vbus->feed(data);
}
