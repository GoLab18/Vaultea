#include "PageManager.h"
#include "Constants.h"
#include "storage/Constants.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>

using namespace vault::storage;
using namespace vault::storage::page;

uint16_t PageManager::freeSpace(const Page &page) {
  const auto &h = page.layout.header;
  return h.upper - h.lower;
}

bool PageManager::hasDeletedSlots(const Page &page) {
  for (const auto &slot : page.layout.slots) {
    if (slot.state == SLOT_DELETED) {
      return true;
    }
  }

  return false;
}

std::optional<SlotId> PageManager::findReusableSlot(const Page &page) {
  for (SlotId i = 0; i < page.layout.slots.size(); i++) {
    if (page.layout.slots[i].state == SLOT_DELETED) {
      return i;
    }
  }

  return std::nullopt;
}

SlotId PageManager::insert(Page &page, const uint8_t *data, uint16_t size) {
  auto &h = page.layout.header;

  bool needsNewSlot = !findReusableSlot(page).has_value();

  uint16_t needed = size + (needsNewSlot ? SLOT_SIZE : 0);

  if (freeSpace(page) < needed && hasDeletedSlots(page)) {
    compact(page);
  }

  if (freeSpace(page) < needed) {
    throw std::runtime_error("page full");
  }

  h.upper -= size;

  uint16_t writeOffset = h.upper;

  std::memcpy(page.data.data() + writeOffset, data, size);

  SlotId slotId;

  auto reusable = findReusableSlot(page);

  if (reusable.has_value()) {
    slotId = *reusable;
  } else {
    slotId = h.slotCount++;

    page.layout.slots.resize(slotId + 1);

    h.lower += SLOT_SIZE;
  }

  page.layout.slots[slotId] =
      Slot{.offset = writeOffset, .size = size, .state = SLOT_USED};

  page.dirty = true;

  return slotId;
}

std::vector<uint8_t> PageManager::read(Page &page, SlotId slotId) {
  if (slotId >= page.layout.slots.size())
    throw std::runtime_error("invalid slot");

  const Slot &s = page.layout.slots[slotId];

  if (s.state == SLOT_DELETED)
    throw std::runtime_error("slot deleted");

  std::vector<uint8_t> out(s.size);
  std::memcpy(out.data(), page.data.data() + s.offset, s.size);

  return out;
}

std::optional<SlotId> PageManager::update(Page &page, SlotId slotId,
                                          const uint8_t *data, uint16_t size) {
  if (slotId >= page.layout.slots.size())
    throw std::runtime_error("invalid slot");

  Slot &s = page.layout.slots[slotId];

  if (s.state == SLOT_DELETED)
    throw std::runtime_error("slot deleted");

  // If fits -> overwrite in place
  if (size <= s.size) {
    std::memcpy(page.data.data() + s.offset, data, size);
    s.size = size;
    page.dirty = true;

    return std::nullopt;
  }

  // If it's too large -> relocate
  s.state = SLOT_DELETED;

  SlotId newSlot = insert(page, data, size);
  page.dirty = true;

  return newSlot;
}

void PageManager::remove(Page &page, SlotId slotId) {
  if (slotId >= page.layout.slots.size())
    throw std::runtime_error("invalid slot");

  page.layout.slots[slotId].state = SLOT_DELETED;
  page.dirty = true;
}

// Sliding compaction
void PageManager::compact(Page &page) {
  auto &h = page.layout.header;

  uint8_t *data = page.data.data();

  std::vector<Slot *> activeSlots;
  activeSlots.reserve(page.layout.slots.size());

  for (auto &slot : page.layout.slots) {
    if (slot.state == SLOT_USED) {
      activeSlots.push_back(&slot);
    }
  }

  std::sort(activeSlots.begin(), activeSlots.end(),
            [](const Slot *a, const Slot *b) { return a->offset > b->offset; });

  uint16_t writeOffset = static_cast<uint16_t>(page.data.size());

  for (Slot *slot : activeSlots) {
    uint16_t size = slot->size;

    writeOffset -= size;

    std::memmove(data + writeOffset, data + slot->offset, size);

    slot->offset = writeOffset;
  }

  h.upper = writeOffset;

  page.dirty = true;
}
