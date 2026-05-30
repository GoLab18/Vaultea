#pragma once

#include "Constants.h"
#include "Page.h"

#include <optional>

using namespace vault::storage::page;

class PageManager {
public:
  static SlotId insert(Page &page, const uint8_t *data, uint16_t size);

  static std::vector<uint8_t> read(Page &page, SlotId slotId);

  static std::optional<SlotId> update(Page &page, SlotId slotId,
                                      const uint8_t *data, uint16_t size);

  static void remove(Page &page, SlotId slotId);

  static void compact(Page &page);

private:
  static uint16_t freeSpace(const Page &page);

  static bool hasDeletedSlots(const Page &page);

  static std::optional<SlotId> findReusableSlot(const Page &page);
};
