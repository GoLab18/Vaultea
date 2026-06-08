#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace vault::storage {

inline constexpr uint32_t VERSION = 1;

inline constexpr std::size_t MAGIC_SIZE = 4;
inline constexpr std::array<uint8_t, MAGIC_SIZE> MAGIC = {'V', 'T', 'E', 'A'};

inline constexpr std::size_t VAULT_PREAMBLE_SIZE = 48;
inline constexpr std::size_t VAULT_HEADER_SIZE = 56;

inline constexpr std::size_t UUID_SIZE = 16;

using RawBytes = std::vector<uint8_t>;

} // namespace vault::storage
