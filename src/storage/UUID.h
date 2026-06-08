#pragma once

#include "storage/Constants.h"

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

using namespace vault::storage;

class UUID {
public:
  UUID() = default;
  explicit UUID(const std::array<uint8_t, UUID_SIZE> &b);

  static UUID random();
  static UUID fromString(std::string_view str);
  static UUID fromRaw(const std::array<uint8_t, UUID_SIZE> &b) {
    return UUID{b};
  }

  std::string toString() const;

  bool operator==(const UUID &other) const noexcept {
    return bytes == other.bytes;
  }

  bool operator!=(const UUID &other) const noexcept {
    return !(*this == other);
  }

  bool operator<(const UUID &other) const noexcept {
    return bytes < other.bytes;
  }

  const std::array<uint8_t, UUID_SIZE> &raw() const;

  std::size_t hash() const noexcept;

private:
  std::array<uint8_t, UUID_SIZE> bytes{};

  static uint8_t hexToByte(char c);
};

namespace std {
template <> struct hash<UUID> {
  std::size_t operator()(const UUID &u) const noexcept { return u.hash(); }
};
} // namespace std
