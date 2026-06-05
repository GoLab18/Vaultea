#pragma once

#include "Constants.h"
#include "PageLayout.h"
#include "storage/Constants.h"

#include <cstdint>
#include <memory>

using namespace vault::storage::page;

struct Page {
  PageId id;
  vault::storage::RawBytes data;

  std::unique_ptr<PageLayoutBase> layout;

  bool dirty = false;
  uint32_t pinCount = 0;
};
