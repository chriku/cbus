#pragma once

#include <iostream>
#include <stdexcept>
#include <stdlib.h>
#include <string>

namespace becker {
  /**
   * \brief Error for assertion fail
   */
  class assertion_failed_error : public std::runtime_error {
  public:
    /**
     * \brief Construct new assertion failed error
     * \param file __FILE__ macro content
     * \param line __LINE__ macro content
     * \param assertion_failed Message content
     */
    assertion_failed_error(const char* file, uint_fast32_t line, std::string assertion_failed)
        : std::runtime_error(std::string(file) + ":" + std::to_string(line) + ": " + assertion_failed) {}
  };
  void bassert(bool condition, const char* file, uint_fast32_t line, std::string assertion_failed = "becker's baking burned") {
    if (!condition) {
      std::cerr << "Becker died in " << file << ":" << line << ": " << assertion_failed << std::endl;
      abort();
      // throw assertion_failed_error(file, line, assertion_failed);
    }
  }
} // namespace becker
