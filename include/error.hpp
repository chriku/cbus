#pragma once

namespace cbus {
  enum class error_code : uint8_t { illegal_function = 1, illegal_data_address = 2, gateway_no_response = 11 };
}
