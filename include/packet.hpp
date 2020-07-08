#pragma once

#include "becker.hpp"
#include <functional>
#include <memory>
#include <string>
#include <variant>

namespace cbus {
  struct not_enough_data {};
  struct packet_error {};
  struct read_coils_response;
  struct read_coils_request;
  struct read_input_registers_request;
  struct read_input_registers_response;

  using single_packet = std::variant<not_enough_data, packet_error, read_coils_response, read_coils_request, read_input_registers_response, read_input_registers_request>;
  enum class function_code {
    invalid = 0,
    read_coils = 1,
    read_discrete_inputs = 2,
    read_multiple_holding_registers = 3,
    read_input_registers = 4,
    write_single_coil = 5,
    write_single_holding_register = 6,
    write_multiple_coils = 15,
    write_multiple_holding_registers = 16
  };

  /**
   * \brief A modbus packet
   */
  struct packet {
    /**
     * \brief conmstruct new packet cointaining Values
     * \param p_transaction_id data
     * \param p_address address field data
     * \param p_function runction code
     */
    packet(const uint16_t p_transaction_id, const uint8_t p_address, const function_code p_function) : transaction_id(p_transaction_id), address(p_address), function(p_function) {}
    /**
     * \brief default copy constructor
     * \param other copy source
     */
    packet(const packet& other) = default;
    /**
     * \brief transaction_id, 0 if not used
     */
    const uint16_t transaction_id;
    /**
     * \brief device address
     */
    const uint8_t address;
    /**
     * \brief Function Code
     */
    const function_code function;
  };
  /**
   * \brief Read single 16bit value
   * \param start_index the first byte in cache_ to read
   * \return the read and converted values
   */
  uint16_t get_u16(const std::string string, const size_t start_index = 0) {
    becker::assert((start_index + 1) < string.size(), __FILE__, __LINE__);
    uint16_t value = 0;
    value |= ((uint8_t)string.at(start_index));
    value <<= 8;
    value |= ((uint8_t)string.at(start_index + 1));
    return value;
  }

  /**
   * \brief Read single 8bit value
   * \param start_index the first byte in cache_ to read
   * \return the read and converted values
   */
  uint8_t get_u8(const std::string string, const size_t start_index = 0) {
    becker::assert(start_index < string.size(), __FILE__, __LINE__);
    return ((uint8_t)string.at(start_index));
  }
  template <typename T> single_packet parse_single_packet(const packet& header, const std::string& content, uint_least64_t& size);
} // namespace cbus
