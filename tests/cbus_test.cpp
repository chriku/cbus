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
  std::shared_ptr<virtual_bus> vbus = std::make_shared<virtual_bus>();
  cbus::bus<virtual_bus> b(vbus, cfg);
  CHECK(b.open());
  vbus->feed("a");
  CHECK(b.open());
  time += 1005;
  CHECK(b.open());
}

TEST_CASE("test simple receive of tcp packet") {
  uint64_t time = 0;
  cbus::config cfg;
  cfg.now = [&time] { return time; };
  cfg.close_on_timeout = true;
  cfg.use_tcp_format = true;
  cfg.is_master = false;
  std::shared_ptr<virtual_bus> vbus = std::make_shared<virtual_bus>();
  cbus::bus<virtual_bus> b(vbus, cfg);
  CHECK(b.open());
  std::string data("\x00\x00\x00\x00\x00\x06\x00\x01\x01\x00\x00\x01", 5);
  vbus->feed(data);
  CHECK(b.open());
}

TEST_CASE("test simple receive of slow multiple tcp packets") {
  uint64_t time = 0;
  cbus::config cfg;
  cfg.now = [&time] { return time; };
  cfg.close_on_timeout = true;
  cfg.use_tcp_format = true;
  cfg.is_master = false;
  std::shared_ptr<virtual_bus> vbus = std::make_shared<virtual_bus>();
  cbus::bus<virtual_bus> b(vbus, cfg);
  CHECK(b.open());
  std::string data("\x00\x00\x00\x00\x00\x06\x00\x01\x01\x00\x00\x01", 12);
  data = data + data + data + data + data + data + data + data;
  for (uint_least32_t i = 0; i < data.size(); i += 13) {
    vbus->feed(data.substr(i, 13));
    time += 500;
    CHECK(b.open());
    if (!b.open())
      std::cerr << b.error_string() << std::endl;
  }
  CHECK(b.open());
}

TEST_CASE("test invalid protocol id") {
  uint64_t time = 0;
  cbus::config cfg;
  cfg.now = [&time] { return time; };
  cfg.close_on_timeout = true;
  cfg.use_tcp_format = true;
  std::shared_ptr<virtual_bus> vbus = std::make_shared<virtual_bus>();
  cbus::bus<virtual_bus> b(vbus, cfg);
  CHECK(b.open());
  std::string data("\x00\x00\x00\x01\x00\x06\x00\x01\x01\x00\x00\x01", 5);
  vbus->feed(data);
}
