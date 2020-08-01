#pragma once

#include "becker.hpp"
#include "config.hpp"
#include "contents.hpp"
#include "packet.hpp"
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <variant>

namespace cbus {

  uint16_t calc_crc(std::string data) {
    uint16_t crc = 0xFFFF;

    for (uint_fast32_t pos = 0; pos < data.size(); pos++) {
      crc ^= (uint16_t)((uint8_t)data.at(pos));
      for (uint_fast8_t i = 8; i != 0; i--) {
        if ((crc & 0x0001) != 0) {
          crc >>= 1;
          crc ^= 0xA001;
        } else
          crc >>= 1;
      }
    }
    return (crc >> 8) | (crc << 8);
  }

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
     * \param packet_emission Callback to be called on incoming packet
     */
    bus(const std::shared_ptr<device_type> device, const config& cfg, const std::function<void(const single_packet&)> packet_emission)
        : device_(device), config_(cfg), packet_emission_(packet_emission) {
      if ((!cfg.is_master) && (!cfg.use_tcp_format)) {
        std::cerr << "Cannot become RTU-Slave" << std::endl;
        abort();
      }
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

    /**
     * \brief send a packet
     * \param packet the packet to send
     * The resulting device->send call always receives excatly one complete package.
     */
    template <typename packet_type> void send(const packet_type& packet) {
      std::string output;
      std::string content = serialize_single_packet<packet_type>(packet);
      if (config_.use_tcp_format) {
        output.append(set_u16(packet.transaction_id));
        output.append(set_u16(0));
        output.append(set_u16(content.size() + 2));
        output.append(set_u8(packet.address));
        output.append(set_u8((uint8_t)packet.function));
        output.append(content);
      } else {
        output.append(set_u8(packet.address));
        output.append(set_u8((uint8_t)packet.function));
        output.append(serialize_single_packet<packet_type>(packet));
        output.append(set_u16(calc_crc(output)));
      }
      device_->send(output);
    }

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
     * \brief parse a single packet
     * \param header the header of the packet
     * \param conhtent the content to use
     */
    single_packet parse_packet(const packet& header, const std::string& content, uint_least64_t& size) {
      if (config_.is_master) {
        switch (header.function) {
        case function_code::read_coils:
          return parse_single_packet<read_coils_response>(header, content, size);
        case function_code::read_input_registers:
          return parse_single_packet<read_input_registers_response>(header, content, size);
        default:
          return packet_error{};
        }
      } else {
        switch (header.function) {
        case function_code::read_coils:
          return parse_single_packet<read_coils_request>(header, content, size);
        case function_code::read_input_registers:
          return parse_single_packet<read_input_registers_request>(header, content, size);
        default:
          return packet_error{};
        }
      }
      return packet_error{};
    }

    /**
     * \brief process single received tcp packet
     * \param pkg the header
     * \param content content string
     * \return true to continue, false to abort reading
     */
    bool process_received_tcp_packet(const packet& pkg, const std::string& content) {
      uint_least64_t read_size = 0;
      if (config_.is_master || (pkg.address == config_.address)) {
        single_packet result = parse_packet(pkg, content, read_size);
        if (std::holds_alternative<packet_error>(result)) {
          close("packet error");
          return false;
        }
        if (std::holds_alternative<not_enough_data>(result)) {
          return false;
        }
        if (read_size != content.size()) {
          close("not enough data read: " + std::to_string(read_size) + "/" + std::to_string(content.size()));
          return false;
        }
        packet_emission_(result);
      }
      return true;
    }

    /**
     * \brief extract single received tcp packet
     * \return true to continue, false to abort reading
     */
    bool extract_single_tcp_packet() {
      if (cache_.size() < 8)
        return false;
      uint16_t transaction_id = get_u16(cache_, 0);
      uint16_t protocol_id = get_u16(cache_, 2);
      if (protocol_id != 0) {
        close("invalid protocol id");
        return false;
      }
      uint16_t length = get_u16(cache_, 4);
      if (length < 2) {
        close("invalid length");
        return false;
      }
      length -= 2;
      uint8_t address = cache_.at(6);
      function_code function = (function_code)cache_.at(7);
      packet pkg(transaction_id, address, function);
      if (cache_.size() < (length + 8))
        return false;
      std::string content = cache_.substr(8, length);
      cache_ = cache_.substr(8 + length);
      if (!process_received_tcp_packet(pkg, content))
        return false;
      return true;
    }

    /**
     * \brief process single received tcp packet
     * \param pkg the header
     * \param content content string
     * \return true to continue, false to abort reading
     */
    bool process_received_rtu_packet(const packet& pkg) {
      uint_least64_t read_size = 0;
      if (config_.is_master || (pkg.address == config_.address)) {
        single_packet result = parse_packet(pkg, cache_.substr(2), read_size);
        if (std::holds_alternative<packet_error>(result)) {
          cache_ = cache_.substr(1);
          return false;
        }
        if (std::holds_alternative<not_enough_data>(result)) {
          return false;
        }
        if (cache_.size() < (2 + read_size + 2)) {
          return false;
        }
        uint16_t read_crc = get_u16(cache_, 2 + read_size);
        if (read_crc != calc_crc(cache_.substr(0, 2 + read_size))) {
          std::cout << std::hex << read_crc << ":" << calc_crc(cache_.substr(0, 2 + read_size)) << std::dec << std::endl;
          cache_ = cache_.substr(1);
          return false;
        }
        cache_ = cache_.substr(2 + read_size + 2);
        packet_emission_(result);
      }
      return true;
    }

    /**
     * \brief extract single received rtu packet
     * \return true to continue, false to abort reading
     */
    bool extract_single_rtu_packet() {
      if (cache_.size() < 2)
        return false;
      uint8_t address = cache_.at(0);
      function_code function = (function_code)cache_.at(1);
      packet pkg(0, address, function);
      if (!process_received_rtu_packet(pkg))
        return false;
      return true;
    }

    /**
     * \brief Read all available tcp packets
     */
    void read_tcp_packets() {
      becker::bassert(config_.use_tcp_format, __FILE__, __LINE__, "calling tcp in rtu mode");
      becker::bassert(cache_.size() > 0, __FILE__, __LINE__, "cache empty");
      while (cache_.size() > 0) {
        if (!extract_single_tcp_packet())
          break;
      }
    }

    /**
     * \brief Read all available rtu packets
     */
    void read_rtu_packets() {
      becker::bassert(!config_.use_tcp_format, __FILE__, __LINE__, "calling rtu in tcp mode");
      becker::bassert(cache_.size() > 0, __FILE__, __LINE__, "cache empty");
      while (cache_.size() > 0) {
        if (!extract_single_rtu_packet())
          break;
      }
    }

    /**
     * \brief feed data into the cache
     * Timeouts are NOT enlarged by the received time to allow a large receive after missing a timeout.
     */
    void feed(const std::string& data) {
      std::cout << "Closed: " << closed_ << std::endl;
      if (closed_)
        return;
      refresh_timeouts(data.size() > 0);
      std::cout << "Closed: " << closed_ << std::endl;
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
    std::shared_ptr<bool> bus_valid_;
    std::shared_ptr<device_type> device_;
    config config_;
    int_least64_t last_byte_received_time_;
    std::string error_string_;
    const std::function<void(const single_packet&)> packet_emission_;
  };
} // namespace cbus
