#pragma once

namespace cbus {
  enum class error_code : uint8_t {
    //
    illegal_function = 1,
    illegal_data_address = 2,
    illegal_data_value = 3,
    slave_device_failure = 4,
    acknowledge = 5,
    slave_device_busy = 6,
    negative_acknowledge = 7,
    memory_parity_error = 8,
    gateway_path_unavailable = 10,
    gateway_no_response = 11
  };
}
