#pragma once

#include "becker.hpp"
#include <functional>
#include <memory>
#include <string>
#include <variant>

namespace cbus {
  struct not_enough_data {};
  struct packet_error;
  struct error_response;
  struct unknown_packet_error;
  struct internal_error;
  struct read_coils_response;
  struct read_coils_request;
  struct read_input_registers_request;
  struct read_input_registers_response;
  struct read_holding_registers_request;
  struct read_holding_registers_response;
  struct write_single_holding_register_request;
  struct write_single_holding_register_response;
  struct write_holding_registers_request;
  struct write_holding_registers_response;

  using single_packet = std::variant<not_enough_data,                        // 0
                                     packet_error,                           // 1
                                     unknown_packet_error,                   // 2
                                     internal_error,                         // 3
                                     read_coils_response,                    // 4
                                     read_coils_request,                     // 5
                                     read_input_registers_response,          // 6
                                     read_input_registers_request,           // 7
                                     read_holding_registers_response,        // 8
                                     read_holding_registers_request,         // 9
                                     error_response,                         // 10
                                     write_single_holding_register_request,  // 11
                                     write_single_holding_register_response, // 12
                                     write_holding_registers_request,        // 13
                                     write_holding_registers_response        // 14
                                     >;
  enum class function_code {
    invalid = 0,
    read_coils = 1,
    read_discrete_inputs = 2,
    read_holding_registers = 3,
    read_input_registers = 4,
    write_single_coil = 5,
    write_single_holding_register = 6,
    write_multiple_coils = 15,
    write_holding_registers = 16
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
  * \brief Error Packet for common error
  */
  struct packet_error : packet {
    packet_error(const uint16_t p_transaction_id, const uint8_t p_address, const function_code p_function) : packet(p_transaction_id, p_address, p_function) {}
    packet_error(const packet& header) : packet(header) {}
  };

  /**
  * \brief Error Packet for internal programming issues
  */
  struct internal_error : packet {
    internal_error(const uint16_t p_transaction_id, const uint8_t p_address, const function_code p_function) : packet(p_transaction_id, p_address, p_function) {}
    internal_error(const packet& header) : packet(header) {}
  };

  /**
  * \brief Error Packet for unknown function code
  */
  struct unknown_packet_error : packet {
    unknown_packet_error(const uint16_t p_transaction_id, const uint8_t p_address, const function_code p_function) : packet(p_transaction_id, p_address, p_function) {}
    unknown_packet_error(const packet& header) : packet(header) {}
  };

  /**
   * \brief Read single 16bit value
   * \param start_index the first byte in cache_ to read
   * \return the read and converted values
   */
  inline uint16_t get_u16(const char* file, uint_fast32_t line, const std::string string, const size_t start_index = 0) {
    becker::bassert((start_index + 1) < string.size(), file, line);
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
  inline uint8_t get_u8(const std::string string, const size_t start_index = 0) {
    becker::bassert(start_index < string.size(), __FILE__, __LINE__);
    return ((uint8_t)string.at(start_index));
  }

  inline std::string set_u8(const uint8_t value) { return std::string((const char*)&value, 1); }
  inline std::string set_u16(const uint16_t value) {
    uint8_t val[2];
    val[0] = ((uint8_t)(value >> 8));
    val[1] = ((uint8_t)value & 0xff);
    return std::string((char*)val, 2);
  }

  template <typename T> single_packet parse_single_packet(const packet& header, const std::string& content, uint_least64_t& size);
  template <typename T> std::string serialize_single_packet(const T& packet);
} // namespace cbus
