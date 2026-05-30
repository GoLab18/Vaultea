#pragma once

#include <cstddef>
#include <cstdint>

namespace vault::storage::page {

inline constexpr uint32_t DEFAULT_PAGE_SIZE = 4096;

inline constexpr std::size_t HEADER_SIZE = 8;
inline constexpr std::size_t SLOT_SIZE = 5;

using PageId = uint64_t;
using SlotId = uint16_t;

} // namespace vault::storage::page
