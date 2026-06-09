#pragma once

#include "Constants.h"
#include "Page.h"
#include "storage/Constants.h"

#include <optional>

using namespace vault::storage;
using namespace vault::storage::page;

enum class UpdateResult { UpdatedInPlace, RequiresRelocation };

class SlottedPageHandler {
public:
  static SlotId insert(Page &page, const uint8_t *data, uint16_t size);
  static RawBytes read(Page &page, SlotId slotId);
  static UpdateResult update(Page &page, SlotId slotId, const uint8_t *data,
                             uint16_t size);
  static bool remove(Page &page, SlotId slotId);
  static void compact(Page &page);

private:
  static uint16_t freeSpace(const Page &page);
  static bool hasDeletedSlots(const Page &page);
  static std::optional<SlotId> findReusableSlot(const Page &page);
};
