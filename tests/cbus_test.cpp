#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "cbus.hpp"
#include "doctest.h"

struct virtual_bus {
  void register_handler(std::function<void(const std::string&)>) { feed = feed; }
  std::function<void(const std::string&)> feed;
};

TEST_CASE("test simple receive of single packet") {
  uint64_t time = 0;
  cbus::config cfg;
  cfg.now = [&time] { return time; };
  std::shared_ptr<virtual_bus> vbus = std::make_shared<virtual_bus>();
  cbus::bus<virtual_bus> b(vbus, cfg);
  CHECK(b.open());
}
