#pragma once

#include "Constants.h"
#include "Page.h"
#include "PageLayout.h"

#include <optional>

using namespace vault::storage;

class PageManager {
public:
  static SlotId insert(Page &page, const uint8_t *data, uint16_t size);

  static std::vector<uint8_t> read(Page &page, SlotId slotId);

  static std::optional<SlotId> update(Page &page, SlotId slotId,
                                      const uint8_t *data, uint16_t size);

  static void remove(Page &page, SlotId slotId);

  static void compact(Page &page);
};
