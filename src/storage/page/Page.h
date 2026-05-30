#pragma once

#include "PageLayout.h"
#include "Constants.h"
#include "storage/Constants.h"

#include <cstdint>

using namespace vault::storage::page;

struct Page {
  PageId id;
  vault::storage::RawBytes data;
  PageLayout layout;

  bool dirty = false;
  uint32_t pinCount = 0;
};
