#pragma once

#include "becker.hpp"
#include <functional>
#include <iostream>
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
  };

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
     * \param transaction_id data
     * \param address address field data
     * \param function runction code
     */
    packet(const uint16_t transaction_id, const uint8_t address, const function_code function) : transaction_id(transaction_id), address(address), function(function) {}
    /**
     * \brief default copy constructor
     * \param other copy source
     */
    packet(const packet& other) = default;
    /**
     * \brief transaction_id, 0 if not used
     */
    const uint16_t transaction_id = 0;
    /**
     * \brief device address
     */
    const uint8_t address = 0;
    /**
     * \brief Function Code
     */
    const function_code function = function_code::invalid;
  };

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
     * \brief construct new read_coils_response
     * \param header containing header struff
     * \param first_coil index of first coil
     * \param coil_count number of coils to request
     */
    read_coils_request(const packet& header, uint16_t first_coil, uint16_t coil_count) : packet(header), first_coil(first_coil), coil_count(coil_count) {}
    /**
     * \brief First Coil index
     */
    uint16_t first_coil;
    /**
     * \brief Number of coils
     */
    uint16_t coil_count;
  };

  struct not_enough_data {};
  struct packet_error {};

  using single_packet = std::variant<not_enough_data, packet_error, read_coils_response, read_coils_request>;

  /**
   * \brief Class describing a single bus.
   * This could be a Modbus-TCP Connection or a Modbus-RTU Handle
   */
  template <typename device_type> class bus {
  public:
    /**
     * \brief Construct a new bus
     * \param device The device to use
     * \param cfg The config to user
     */
    bus(const std::shared_ptr<device_type> device, const config& cfg) : device_(device), config_(cfg) {
      if ((!cfg.is_master) && (!cfg.use_tcp_format))
        throw std::invalid_argument("Cannot become RTU-Slave");
      bus_valid_ = std::make_shared<bool>(true);
      init_bus_handler();
    }
    ~bus() { *bus_valid_ = false; }

    /**
     * \brief check if bus is still open
     * \return if the bus is still open or should be closed
     */
    bool open() const { return !closed_; }

    /**
     * \brief close the bus
     */
    void close() { close("user"); }

    /**
     * \brief refresh timeouts
     */
    void refresh_timeouts() { refresh_timeouts(false); };

    /**
     * \brief get error string
     * \return the last error message or an empty string
     */
    std::string error_string() const { return error_string_; }

  private:
    /**
     * \brief close the bus
     * \param the message to use as error string
     */
    void close(std::string message) {
      if (!closed_)
        error_string_ = message;
      closed_ = true;
    }

    /**
     * \brief Refresh timeouts
     * \param if the last receive time sould be updated
     */
    void refresh_timeouts(const bool bytes_received) {
      int_least64_t new_time = config_.now();
      int_least64_t difference = new_time - last_byte_received_time_;
      if (bytes_received)
        last_byte_received_time_ = new_time;
      if ((difference > config_.silence_timeout) && (cache_.size() > 0)) {
        if (config_.close_on_timeout) {
          close("timeout");
          return;
        } else {
          cache_.clear();
        }
      }
    }

    /**
     * \register the receive handler into the bus
     */
    void init_bus_handler() {
      std::shared_ptr<bool> bus_valid = bus_valid_;
      device_->register_handler([bus_valid, this](const std::string& data) {
        if (*bus_valid)
          feed(data);
      });
    }

    /**
     * \brief Read single 16bit value
     * \param start_index the first byte in cache_ to read
     * \return the read and converted values
     */
    static uint16_t get_u16(const std::string string, const size_t start_index = 0) {
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
    static uint8_t get_u8(const std::string string, const size_t start_index = 0) {
      becker::assert(start_index < string.size(), __FILE__, __LINE__);
      return ((uint8_t)string.at(start_index));
    }

    /**
     * \brief Read single 16bit value
     * \param start_index the first byte in cache_ to read
     * \return the read and converted values
     */
    uint16_t get_u16(const size_t start_index) const { return get_u16(cache_, start_index); }

    /**
     * \brief Read single 8bit value
     * \param start_index the first byte in cache_ to read
     * \return the read and converted values
     */
    uint8_t get_u8(const size_t start_index) const { return get_u8(cache_, start_index); }

    /**
     * \brief parse a single packet
     * \param header the header of the packet
     * \param conhtent the content to use
     */
    single_packet parse_packet(const packet& header, const std::string& content, uint_least64_t& size) {
      if (config_.is_master) {
        switch (header.function) {
        case function_code::read_coils: {
          if (content.size() < 1)
            return not_enough_data{};
          uint8_t len = get_u8(content, 0);
          if (content.size() < (1 + len))
            return not_enough_data{};
          size = len + 1;
          return read_coils_response(header, content.substr(1));
        } break;
        default:
          return packet_error{};
        }
      } else {
        switch (header.function) {
        case function_code::read_coils: {
          if (content.size() < 4)
            return not_enough_data{};
          uint16_t first_coil = get_u16(content, 0);
          uint16_t coil_count = get_u16(content, 2);
          size = 4;
          return read_coils_request(header, first_coil, coil_count);
        } break;
        default:
          return packet_error{};
        }
      }
      return packet_error{};
    }

    /**
     * \brief Read all available tcp packets
     */
    void read_tcp_packets() {
      becker::assert(config_.use_tcp_format, __FILE__, __LINE__, "calling tcp in rtu mode");
      becker::assert(cache_.size() > 0, __FILE__, __LINE__, "cache empty");
      while (true) {
        if (cache_.size() < 8)
          break;
        uint16_t transaction_id = get_u16(0);
        uint16_t protocol_id = get_u16(2);
        if (protocol_id != 0) {
          close("invalid protocol id");
          return;
        }
        uint16_t length = get_u16(4);
        if (length < 2) {
          close("invalid length");
          return;
        }
        length -= 2;
        uint8_t address = cache_.at(6);
        function_code function = (function_code)cache_.at(7);
        packet pkg(transaction_id, address, function);
        if (cache_.size() < (length + 8))
          break;
        std::string content = cache_.substr(8, length);
        cache_ = cache_.substr(8 + length);
        uint_least64_t read_size = 0;
        if (config_.is_master || (pkg.address == config_.address)) {
          single_packet result = parse_packet(pkg, content, read_size);
          if (std::holds_alternative<packet_error>(result)) {
            close("packet error");
            return;
          }
          if (std::holds_alternative<not_enough_data>(result)) {
            break;
          }
          if (read_size != content.size()) {
            close("not enough data read: " + std::to_string(read_size) + "/" + std::to_string(content.size()));
            return;
          }
        }
      }
    }

    /**
     * \brief Read all available rtu packets
     */
    void read_rtu_packets() {
      becker::assert(!config_.use_tcp_format, __FILE__, __LINE__, "calling rtu in tcp mode");
      becker::assert(cache_.size() > 0, __FILE__, __LINE__, "cache empty");
    }

    /**
     * \brief feed data into the cache
     * Timeouts are NOT enlarged by the received time to allow a large receive after missing a timeout.
     */
    void feed(const std::string& data) {
      if (closed_)
        return;
      refresh_timeouts(data.size() > 0);
      if (closed_)
        return;
      cache_.append(data);
      if (cache_.size() > 0) {
        if (config_.use_tcp_format)
          read_tcp_packets();
        else
          read_rtu_packets();
      }
    }

    std::string cache_;
    bool closed_ = false;
    config config_;
    std::shared_ptr<bool> bus_valid_;
    std::shared_ptr<device_type> device_;
    int_least64_t last_byte_received_time_;
    std::string error_string_;
  };
} // namespace cbus
