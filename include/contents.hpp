#pragma once

#include "becker.hpp"
#include "error.hpp"
#include "packet.hpp"
#include <functional>
#include <memory>
#include <string.h>
#include <string>
#include <variant>

namespace cbus {
  /**
   * \brief response for function code 1 read coils
   */
  struct read_coils_response : packet {
    /**
     * \brief create nmew coils response
     * \param transaction_id The id of the transaction
     * \param address The address of the target
     * \param  coil_data The content
     */
    read_coils_response(const uint16_t transaction_id, uint8_t address, std::vector<bool> coil_data)
        : packet(transaction_id, address, function_code::read_coils), coil_data(coil_data) {}
    /**
     * \brief construct new read_coils_response
     * \param header containing header struff
     * \param coil_data string describing the content of the coils
     */
    read_coils_response(const packet& header, std::vector<bool> coil_data) : packet(header), coil_data(coil_data) {}
    /**
     * \brief Value of each coil/discrete input is binary (0 for off, 1 for on). First requested coil/discrete input is stored as least significant bit of first byte in reply.
     * If number of coils/discrete inputs is not a multiple of 8, most significant bit(s) of last byte will be stuffed with zeros.
     * For example, if eleven coils are requested, two bytes of values are needed. Suppose states of those successive coils are on, off, on, off, off, on, on, on, off, on, on, then
     * the response will be 02 E5 06 in hexadecimal.
     */
    std::vector<bool> coil_data;
  };

  /**
   * \brief response for function code 1 read coils
   */
  struct read_coils_request : packet {
    /**
     * \brief create nmew coils response
     * \param transaction_id The id of the transaction
     * \param address The address of the target
     * \param first_coil The content
     * \param coil_count The content
     */
    read_coils_request(const uint16_t transaction_id, uint8_t address, uint16_t first_coil, uint16_t coil_count)
        : packet(transaction_id, address, function_code::read_coils), first_coil(first_coil), coil_count(coil_count) {}
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
     * \brief create nmew coils response
     * \param transaction_id The id of the transaction
     * \param address The address of the target
     * \param register_data The content
     */
    read_input_registers_response(const uint16_t transaction_id, uint8_t address, std::vector<uint16_t> register_data)
        : packet(transaction_id, address, function_code::read_input_registers), register_data(register_data) {}
    /**
     * \brief construct new read_registers_response
     * \param header containing header struff
     * \param register_data string describing the content of the registers
     */
    read_input_registers_response(const packet& header, std::vector<uint16_t> register_data) : packet(header), register_data(register_data) {}
    /**
     * \brief Value of each register
     */
    std::vector<uint16_t> register_data;
  };

  /**
   * \brief response for function code 4 read input register
   */
  struct read_input_registers_request : packet {
    /**
     * \brief create nmew coils response
     * \param transaction_id The id of the transaction
     * \param address The address of the target
     * \param first_register The content
     * \param register_count The content
     */
    read_input_registers_request(const uint16_t transaction_id, uint8_t address, uint16_t first_register, uint16_t register_count)
        : packet(transaction_id, address, function_code::read_input_registers), first_register(first_register), register_count(register_count) {}
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

  /**
   * \brief response for function code 1 read input registers
   */
  struct read_holding_registers_response : packet {
    /**
     * \brief create nmew coils response
     * \param transaction_id The id of the transaction
     * \param address The address of the target
     * \param register_data The content
     */
    read_holding_registers_response(const uint16_t transaction_id, uint8_t address, std::vector<uint16_t> register_data)
        : packet(transaction_id, address, function_code::read_holding_registers), register_data(register_data) {}
    /**
     * \brief construct new read_registers_response
     * \param header containing header struff
     * \param register_data string describing the content of the registers
     */
    read_holding_registers_response(const packet& header, std::vector<uint16_t> register_data) : packet(header), register_data(register_data) {}
    /**
     * \brief Value of each register
     */
    std::vector<uint16_t> register_data;
  };

  /**
   * \brief response for function code 4 read input register
   */
  struct read_holding_registers_request : packet {
    /**
     * \brief create nmew coils response
     * \param transaction_id The id of the transaction
     * \param address The address of the target
     * \param first_register The content
     * \param register_count The content
     */
    read_holding_registers_request(const uint16_t transaction_id, uint8_t address, uint16_t first_register, uint16_t register_count)
        : packet(transaction_id, address, function_code::read_holding_registers), first_register(first_register), register_count(register_count) {}
    /**
     * \brief construct new register_registers_request
     * \param header containing header struff
     * \param first_register index of first register
     * \param register_count number of registers to request
     */
    read_holding_registers_request(const packet& header, const uint16_t first_register, const uint16_t register_count)
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

  /**
   * \brief response for function code 16 write holding registers
   */
  struct write_holding_registers_response : packet {
    /**
     * \brief create nmew coils response
     * \param transaction_id The id of the transaction
     * \param address The address of the target
     * \param register_data The content
     */
    write_holding_registers_response(const uint16_t transaction_id, uint8_t address, uint16_t first_register, uint16_t register_count)
        : packet(transaction_id, address, function_code::write_holding_registers), first_register(first_register), register_count(register_count) {}
    /**
     * \brief construct new read_registers_response
     * \param header containing header struff
     * \param register_data string describing the content of the registers
     */
    write_holding_registers_response(const packet& header, uint16_t first_register, uint16_t register_count)
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

  /**
   * \brief response for function code 4 read input register
   */
  struct write_holding_registers_request : packet {
    /**
     * \brief create nmew coils response
     * \param transaction_id The id of the transaction
     * \param address The address of the target
     * \param first_register The content
     * \param register_count The content
     */
    write_holding_registers_request(const uint16_t transaction_id, uint8_t address, uint16_t first_register, std::vector<uint16_t> register_content)
        : packet(transaction_id, address, function_code::write_holding_registers), first_register(first_register), register_content(register_content) {}
    /**
     * \brief construct new register_registers_request
     * \param header containing header struff
     * \param first_register index of first register
     * \param register_count number of registers to request
     */
    write_holding_registers_request(const packet& header, const uint16_t first_register, std::vector<uint16_t> register_content)
        : packet(header), first_register(first_register), register_content(register_content) {}
    /**
     * \brief First Coil index
     */
    uint16_t first_register;
    /**
     * \brief Number of registers
     */
    std::vector<uint16_t> register_content;
  };

  /**
   * \brief response for function code 16 write holding registers
   */
  struct write_single_holding_register_response : packet {
    /**
     * \brief create nmew coils response
     * \param transaction_id The id of the transaction
     * \param address The address of the target
     * \param register_data The content
     */
    write_single_holding_register_response(const uint16_t transaction_id, uint8_t address, uint16_t register_index, uint16_t register_value)
        : packet(transaction_id, address, function_code::write_single_holding_register), register_index(register_index), register_value(register_value) {}
    /**
     * \brief construct new read_registers_response
     * \param header containing header struff
     * \param register_data string describing the content of the registers
     */
    write_single_holding_register_response(const packet& header, uint16_t register_index, uint16_t register_value)
        : packet(header), register_index(register_index), register_value(register_value) {}
    uint16_t register_index;
    uint16_t register_value;
  };

  /**
   * \brief response for function code 4 read input register
   */
  struct write_single_holding_register_request : packet {
    /**
     * \brief create nmew coils response
     * \param transaction_id The id of the transaction
     * \param address The address of the target
     * \param first_register The content
     * \param register_count The content
     */
    write_single_holding_register_request(const uint16_t transaction_id, uint8_t address, uint16_t register_index, uint16_t register_value)
        : packet(transaction_id, address, function_code::write_single_holding_register), register_index(register_index), register_value(register_value) {}
    /**
     * \brief construct new register_registers_request
     * \param header containing header struff
     * \param first_register index of first register
     * \param register_count number of registers to request
     */
    write_single_holding_register_request(const packet& header, uint16_t register_index, uint16_t register_value)
        : packet(header), register_index(register_index), register_value(register_value) {}
    uint16_t register_index;
    uint16_t register_value;
  };

  /**
   * \brief response for function code 4 read input register
   */
  struct error_response : packet {
    /**
     * \brief create nmew coils response
     * \param transaction_id The id of the transaction
     * \param address The address of the target
     * \param first_register The content
     * \param register_count The content
     */
    error_response(const uint16_t transaction_id, uint8_t address, function_code function, error_code ec)
        : packet(transaction_id, address, static_cast<function_code>(static_cast<uint8_t>(function) | 0x80)), error(ec) {}
    error_response(const packet& header, const error_code ec) : packet(header), error(ec) {}
    error_code error;
  };

  template <> inline single_packet parse_single_packet<read_coils_response>(const packet& header, const std::string& content, uint_least64_t& size) {
    if (content.size() < 1)
      return not_enough_data{};
    uint8_t len = get_u8(content, 0);
    if (content.size() < (1 + len))
      return not_enough_data{};
    size = len + 1;
    std::string raw_reg_data = content.substr(1, len);
    std::vector<bool> response_data;
    for (int8_t byte : raw_reg_data) {
      uint8_t value = byte;
      for (uint_fast8_t i = 0; i < 8; i++)
        response_data.push_back((value & (1 << i)) != 0);
    }
    return read_coils_response(header, response_data);
  }

  template <> inline single_packet parse_single_packet<read_coils_request>(const packet& header, const std::string& content, uint_least64_t& size) {
    if (content.size() < 4)
      return not_enough_data{};
    uint16_t first_coil = get_u16(__FILE__, __LINE__, content, 0);
    uint16_t coil_count = get_u16(__FILE__, __LINE__, content, 2);
    size = 4;
    return read_coils_request(header, first_coil, coil_count);
  }

  template <> inline single_packet parse_single_packet<read_input_registers_response>(const packet& header, const std::string& content, uint_least64_t& size) {
    if (content.size() < 1)
      return not_enough_data{};
    uint8_t len = get_u8(content, 0);
    if (content.size() < (1 + len))
      return not_enough_data{};
    if ((len % 2) != 0)
      return packet_error{header};
    size = len + 1;
    std::string u16_arr = content.substr(1, len);
    std::vector<uint16_t> nd;
    for (uint_fast32_t i = 0; i < u16_arr.size(); i += 2) {
      nd.push_back(get_u16(__FILE__, __LINE__, u16_arr, i));
    }
    return read_input_registers_response(header, nd);
  }

  template <> inline single_packet parse_single_packet<read_input_registers_request>(const packet& header, const std::string& content, uint_least64_t& size) {
    if (content.size() < 4)
      return not_enough_data{};
    uint16_t first_register = get_u16(__FILE__, __LINE__, content, 0);
    uint16_t register_count = get_u16(__FILE__, __LINE__, content, 2);
    size = 4;
    return read_input_registers_request(header, first_register, register_count);
  }

  template <> inline single_packet parse_single_packet<read_holding_registers_response>(const packet& header, const std::string& content, uint_least64_t& size) {
    if (content.size() < 1)
      return not_enough_data{};
    uint8_t len = get_u8(content, 0);
    if (content.size() < (1 + len))
      return not_enough_data{};
    if ((len % 2) != 0)
      return packet_error{header};
    size = len + 1;
    std::string u16_arr = content.substr(1, len);
    std::vector<uint16_t> nd;
    for (uint_fast32_t i = 0; i < u16_arr.size(); i += 2) {
      nd.push_back(get_u16(__FILE__, __LINE__, u16_arr, i));
    }
    return read_holding_registers_response(header, nd);
  }

  template <> inline single_packet parse_single_packet<read_holding_registers_request>(const packet& header, const std::string& content, uint_least64_t& size) {
    if (content.size() < 4)
      return not_enough_data{};
    uint16_t first_register = get_u16(__FILE__, __LINE__, content, 0);
    uint16_t register_count = get_u16(__FILE__, __LINE__, content, 2);
    size = 4;
    return read_holding_registers_request(header, first_register, register_count);
  }

  template <> inline single_packet parse_single_packet<write_holding_registers_response>(const packet& header, const std::string& content, uint_least64_t& size) {
    if (content.size() < 4)
      return not_enough_data{};
    uint16_t first_register = get_u16(__FILE__, __LINE__, content, 0);
    uint16_t register_count = get_u16(__FILE__, __LINE__, content, 2);
    size = 4;
    return write_holding_registers_response(header, first_register, register_count);
  }

  template <> inline single_packet parse_single_packet<write_holding_registers_request>(const packet& header, const std::string& content, uint_least64_t& size) {
    if (content.size() < 5)
      return not_enough_data{};
    uint16_t first_register = get_u16(__FILE__, __LINE__, content, 0);
    uint16_t register_count = get_u16(__FILE__, __LINE__, content, 2);
    uint8_t len = get_u8(content, 4);
    if ((len % 2) != 0)
      return packet_error{header};
    if (content.size() < (5 + len))
      return not_enough_data{};
    size = len + 5;
    std::string u16_arr = content.substr(5, len);
    std::vector<uint16_t> nd;
    for (uint_fast32_t i = 0; i < u16_arr.size(); i += 2) {
      nd.push_back(get_u16(__FILE__, __LINE__, u16_arr, i));
    }
    if (register_count != nd.size())
      return internal_error(header);
    return write_holding_registers_request(header, first_register, nd);
  }

  template <> inline single_packet parse_single_packet<write_single_holding_register_response>(const packet& header, const std::string& content, uint_least64_t& size) {
    if (content.size() < 4)
      return not_enough_data{};
    uint16_t first_register = get_u16(__FILE__, __LINE__, content, 0);
    uint16_t register_count = get_u16(__FILE__, __LINE__, content, 2);
    size = 4;
    return write_single_holding_register_response(header, first_register, register_count);
  }

  template <> inline single_packet parse_single_packet<write_single_holding_register_request>(const packet& header, const std::string& content, uint_least64_t& size) {
    if (content.size() < 4)
      return not_enough_data{};
    uint16_t first_register = get_u16(__FILE__, __LINE__, content, 0);
    uint16_t register_count = get_u16(__FILE__, __LINE__, content, 2);
    size = 4;
    return write_single_holding_register_request(header, first_register, register_count);
  }

  template <> inline std::string serialize_single_packet<read_input_registers_request>(const read_input_registers_request& packet) {
    return set_u16(packet.first_register) + set_u16(packet.register_count);
  }
  template <> inline std::string serialize_single_packet<read_input_registers_response>(const read_input_registers_response& packet) {
    std::string ret = set_u8(packet.register_data.size() * 2);
    for (uint16_t v : packet.register_data)
      ret += set_u16(v);
    return ret;
  }
  template <> inline std::string serialize_single_packet<read_holding_registers_request>(const read_holding_registers_request& packet) {
    return set_u16(packet.first_register) + set_u16(packet.register_count);
  }
  template <> inline std::string serialize_single_packet<read_holding_registers_response>(const read_holding_registers_response& packet) {
    std::string ret = set_u8(packet.register_data.size() * 2);
    for (uint16_t v : packet.register_data)
      ret += set_u16(v);
    return ret;
  }
  template <> inline std::string serialize_single_packet<write_holding_registers_request>(const write_holding_registers_request& packet) {
    std::string ret = set_u16(packet.first_register) + set_u16(packet.register_content.size()) + set_u8(packet.register_content.size() * 2);
    for (uint16_t v : packet.register_content)
      ret += set_u16(v);
    return ret;
  }
  template <> inline std::string serialize_single_packet<write_single_holding_register_request>(const write_single_holding_register_request& packet) {
    return set_u16(packet.register_index) + set_u16(packet.register_value);
  }
  template <> inline std::string serialize_single_packet<write_single_holding_register_response>(const write_single_holding_register_response& packet) {
    return set_u16(packet.register_index) + set_u16(packet.register_value);
  }
  template <> inline std::string serialize_single_packet<write_holding_registers_response>(const write_holding_registers_response& packet) {
    return set_u16(packet.first_register) + set_u16(packet.register_count);
  }
  template <> inline std::string serialize_single_packet<read_coils_request>(const read_coils_request& packet) { return set_u16(packet.first_coil) + set_u16(packet.coil_count); }
  template <> inline std::string serialize_single_packet<read_coils_response>(const read_coils_response& packet) {
    std::string ret = set_u8((packet.coil_data.size() + 7) / 8);
    std::vector<bool> data = packet.coil_data;
    while (data.size() % 8)
      data.push_back(false);
    for (uint_least32_t i = 0; i < data.size(); i += 8) {
      uint8_t b = 0;
      for (uint8_t j = 0; j < 8; j++)
        if (data.at(i + j))
          b |= 1 << j;
    }
    return ret;
  }
  template <> inline std::string serialize_single_packet<error_response>(const error_response& packet) { return set_u8(static_cast<uint8_t>(packet.error)); }
} // namespace cbus
