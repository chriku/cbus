#pragma once

#include "becker.hpp"
#include "packet.hpp"
#include <functional>
#include <memory>
#include <string>
#include <variant>

namespace cbus {
  /**
   * \brief response for function code 1 read coils
   */
  struct read_coils_response : packet {
    /**
     * \brief construct new read_coils_response
     * \param header containing header struff
     * \param coil_data string describing the content of the coils
     */
    read_coils_response(const packet& header, std::string coil_data) : packet(header), coil_data(coil_data) {}
    /**
     * \brief Value of each coil/discrete input is binary (0 for off, 1 for on). First requested coil/discrete input is stored as least significant bit of first byte in reply.
     * If number of coils/discrete inputs is not a multiple of 8, most significant bit(s) of last byte will be stuffed with zeros.
     * For example, if eleven coils are requested, two bytes of values are needed. Suppose states of those successive coils are on, off, on, off, off, on, on, on, off, on, on, then
     * the response will be 02 E5 06 in hexadecimal.
     */
    std::string coil_data;
  };

  /**
   * \brief response for function code 1 read coils
   */
  struct read_coils_request : packet {
    /**
     * \brief construct new read_coils_request
     * \param header containing header struff
     * \param first_coil index of first coil
     * \param coil_count number of coils to request
     */
    read_coils_request(const packet& header, const uint16_t first_coil, const uint16_t coil_count) : packet(header), first_coil(first_coil), coil_count(coil_count) {}
    /**
     * \brief First Coil index
     */
    uint16_t first_coil;
    /**
     * \brief Number of coils
     */
    uint16_t coil_count;
  };

  /**
   * \brief response for function code 1 read input registers
   */
  struct read_input_registers_response : packet {
    /**
     * \brief construct new read_registers_response
     * \param header containing header struff
     * \param register_data string describing the content of the registers
     */
    read_input_registers_response(const packet& header, std::string register_data) : packet(header), register_data(register_data) {}
    /**
     * \brief Value of each register
     */
    std::string register_data;
  };

  /**
   * \brief response for function code 4 read input register
   */
  struct read_input_registers_request : packet {
    /**
     * \brief construct new register_registers_request
     * \param header containing header struff
     * \param first_register index of first register
     * \param register_count number of registers to request
     */
    read_input_registers_request(const packet& header, const uint16_t first_register, const uint16_t register_count)
        : packet(header), first_register(first_register), register_count(register_count) {}
    /**
     * \brief First Coil index
     */
    uint16_t first_register;
    /**
     * \brief Number of registers
     */
    uint16_t register_count;
  };

  template <> single_packet parse_single_packet<read_coils_response>(const packet& header, const std::string& content, uint_least64_t& size) {
    if (content.size() < 1)
      return not_enough_data{};
    uint8_t len = get_u8(content, 0);
    if (content.size() < (1 + len))
      return not_enough_data{};
    size = len + 1;
    return read_coils_response(header, content.substr(1, len));
  }

  template <> single_packet parse_single_packet<read_coils_request>(const packet& header, const std::string& content, uint_least64_t& size) {
    if (content.size() < 4)
      return not_enough_data{};
    uint16_t first_coil = get_u16(content, 0);
    uint16_t coil_count = get_u16(content, 2);
    size = 4;
    return read_coils_request(header, first_coil, coil_count);
  }

  template <> single_packet parse_single_packet<read_input_registers_response>(const packet& header, const std::string& content, uint_least64_t& size) {
    if (content.size() < 1)
      return not_enough_data{};
    uint8_t len = get_u8(content, 0);
    if (content.size() < (1 + len))
      return not_enough_data{};
    size = len + 1;
    return read_input_registers_response(header, content.substr(1, len));
  }

  template <> single_packet parse_single_packet<read_input_registers_request>(const packet& header, const std::string& content, uint_least64_t& size) {
    if (content.size() < 4)
      return not_enough_data{};
    uint16_t first_register = get_u16(content, 0);
    uint16_t register_count = get_u16(content, 2);
    size = 4;
    return read_input_registers_request(header, first_register, register_count);
  }
} // namespace cbus
