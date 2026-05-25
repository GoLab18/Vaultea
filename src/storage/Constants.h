#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace vault::storage {

inline constexpr uint32_t VERSION = 1;

inline constexpr std::size_t MAGIC_SIZE = 4;
inline constexpr std::array<uint8_t, MAGIC_SIZE> MAGIC = {'V', 'T', 'E', 'A'};

inline constexpr std::size_t UUID_SIZE = 16;

inline constexpr uint32_t DEFAULT_PAGE_SIZE = 4096;

using PageId = uint64_t;

using RawBytes = std::vector<uint8_t>;

} // namespace vault::storage
