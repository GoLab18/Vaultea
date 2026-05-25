#include "UUID.h"

#include <sodium/randombytes.h>
#include <stdexcept>

static constexpr char HEX[] = "0123456789abcdef";

UUID::UUID(const std::array<uint8_t, vault::storage::UUID_SIZE> &b)
    : bytes(b) {}

UUID UUID::random() {
  std::array<uint8_t, vault::storage::UUID_SIZE> b{};

  randombytes_buf(b.data(), b.size());

  // UUID v4 rules:
  // - version bits (4 bits)
  // - variant bits (RFC 4122)
  b[6] = (b[6] & 0x0F) | 0x40;
  b[8] = (b[8] & 0x3F) | 0x80;

  return UUID{b};
}

uint8_t UUID::hexToByte(char c) {
  if (c >= '0' && c <= '9')
    return static_cast<uint8_t>(c - '0');
  if (c >= 'a' && c <= 'f')
    return static_cast<uint8_t>(c - 'a' + 10);
  if (c >= 'A' && c <= 'F')
    return static_cast<uint8_t>(c - 'A' + 10);

  throw std::runtime_error("Invalid UUID hex character");
}

UUID UUID::fromString(std::string_view str) {
  constexpr std::size_t STR_SIZE = 36;

  if (str.size() != STR_SIZE) {
    throw std::runtime_error("Invalid UUID length");
  }

  std::array<uint8_t, vault::storage::UUID_SIZE> bytes{};

  int j = 0;

  for (std::size_t i = 0; i < STR_SIZE; ++i) {
    if (str[i] == '-') {
      continue;
    }

    if (j >= static_cast<int>(vault::storage::UUID_SIZE)) {
      throw std::runtime_error("UUID parsing overflow");
    }

    if (i + 1 >= STR_SIZE || str[i + 1] == '-') {
      throw std::runtime_error("Invalid UUID format");
    }

    uint8_t high = hexToByte(str[i]);
    uint8_t low = hexToByte(str[i + 1]);

    bytes[j++] = (high << 4) | low;
    ++i;
  }

  if (j != static_cast<int>(vault::storage::UUID_SIZE)) {
    throw std::runtime_error("UUID parsing failed");
  }

  return UUID{bytes};
}

static UUID fromRaw(const std::array<uint8_t, vault::storage::UUID_SIZE> &b) {
  return UUID{b};
}

std::string UUID::toString() const {
  std::string out(36, '\0');

  std::size_t j = 0;

  for (std::size_t i = 0; i < vault::storage::UUID_SIZE; ++i) {

    if (i == 4 || i == 6 || i == 8 || i == 10) {
      out[j++] = '-';
    }

    out[j++] = HEX[bytes[i] >> 4];
    out[j++] = HEX[bytes[i] & 0x0F];
  }

  return out;
}

bool UUID::operator==(const UUID &other) const { return bytes == other.bytes; }

bool UUID::operator!=(const UUID &other) const { return !(*this == other); }

const std::array<uint8_t, vault::storage::UUID_SIZE> &UUID::raw() const {
  return bytes;
}

std::size_t UUID::hash() const noexcept {

  // FNV-1a
  std::size_t h = 1469598103934665603ull;
  for (auto b : bytes) {
    h ^= b;
    h *= 1099511628211ull;
  }
  return h;
}
