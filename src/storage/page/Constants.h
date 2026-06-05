#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

namespace vault::storage::page {

inline constexpr uint32_t DEFAULT_PAGE_SIZE = 4096;

inline constexpr std::size_t HEADER_SIZE = 23;
inline constexpr std::size_t SLOT_SIZE = 5;

using PageId = uint64_t;
using SlotId = uint16_t;

inline constexpr PageId INVALID_PAGE = std::numeric_limits<PageId>::max();

} // namespace vault::storage::page
