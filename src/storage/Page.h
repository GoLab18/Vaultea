#pragma once

#include "Constants.h"
#include "PageLayout.h"
#include <array>
#include <cstdint>
#include <vector>

struct Page {
  vault::storage::PageId id;
  vault::storage::RawBytes data;
  PageLayout layout;

  bool dirty = false;
  uint32_t pinCount = 0;
};
