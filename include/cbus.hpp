#pragma once

#include <functional>
#include <memory>

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
     * \brief The time that is subtracted from the complete telegram for each byte
     */
    int_least64_t byte_time = 1;
    /**
     * \brief Close the bus on a timeout or just discard the cache
     */
    bool close_on_timeout = false;
  };
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
    void close() { closed_ = true; }

  private:
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

    int_least64_t update_timeout(const size_t data_length) {
      int_least64_t new_time = config_.now();
      int_least64_t difference = (new_time - last_byte_received_time_) - (data_length * config_.byte_time);
      last_byte_received_time_ = new_time;
      return difference;
    }

    /**
     * \brief feed data into the cache
     * Timeouts are enlarged by the received time to allow a large receive after missing a timeout.
     */
    void feed(const std::string& data) {
      if (closed_)
        return;
      int_least64_t difference = update_timeout(data.size());
      if (difference > config_.silence_timeout) {
        if (config_.close_on_timeout) {
          close();
          return;
        } else {
          cache_.clear();
        }
      }
      cache_.append(data);
    }

    std::string cache_;
    bool closed_ = false;
    config config_;
    std::shared_ptr<bool> bus_valid_;
    std::shared_ptr<device_type> device_;
    int_least64_t last_byte_received_time_;
  };
} // namespace cbus
