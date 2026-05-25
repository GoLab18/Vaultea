#pragma once

#include "Constants.h"
#include <array>
#include <cstdint>
#include <vector>

struct Page {
  vault::storage::PageId id;
  vault::storage::RawBytes data;

  bool dirty = false;
  uint32_t pinCount = 0;
};
