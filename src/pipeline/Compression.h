#pragma once

#include <cstdint>
#include <vector>

class Compression {
public:
  static std::vector<uint8_t> compress(const std::vector<uint8_t> &input);
  static std::vector<uint8_t> decompress(const std::vector<uint8_t> &input);
};
