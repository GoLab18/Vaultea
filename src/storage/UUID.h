#pragma once

#include "storage/Constants.h"

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

class UUID {
public:
  static UUID random();
  static UUID fromString(std::string_view str);

  std::string toString() const;

  bool operator==(const UUID &other) const;
  bool operator!=(const UUID &other) const;

  const std::array<uint8_t, vault::storage::UUID_SIZE> &raw() const;

  std::size_t hash() const noexcept;

private:
  std::array<uint8_t, vault::storage::UUID_SIZE> bytes{};

  UUID() = default;
  explicit UUID(const std::array<uint8_t, vault::storage::UUID_SIZE> &b);

  static uint8_t hexToByte(char c);
};

namespace std {
template <> struct hash<UUID> {
  std::size_t operator()(const UUID &u) const noexcept { return u.hash(); }
};
} // namespace std
