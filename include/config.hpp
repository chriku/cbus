#pragma once

#include "becker.hpp"
#include <functional>
#include <memory>
#include <string>
#include <variant>

namespace cbus {
  /**
   * \brief a modbus bus config
   */
  struct config {
    /**
     * \brief A lampbda returning the current time in the same arbitary timne unit as the rest inside this confitg
     */
    std::function<int_least64_t()> now;

    /**
     * \brief The time silence is allowed on the bus before the content is discarded/connection is closed
     */
    int_least64_t silence_timeout = 1000;

    /**
     * \brief Close the bus on a timeout or just discard the cache
     */
    bool close_on_timeout = false;

    /**
     * \brief Use Tcp Format
     */
    bool use_tcp_format = true;

    /**
     * \brief is master
     */
    bool is_master = false;

    /**
     * \brief address of this node
     */
    uint8_t address;

    /**
     * \brief Close socket if any kind of error occurs
     */
    bool close_on_error=false;
  };
} // namespace cbus
