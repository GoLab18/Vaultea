#include "PageManager.h"
#include "Constants.h"
#include "storage/Constants.h"

#include <cstring>
#include <stdexcept>

using namespace vault::storage;
using namespace vault::storage::page;

uint16_t PageManager::freeSpace(const Page &page) {
  const auto &h = page.layout.header;
  return (h.upper - h.lower);
}

SlotId PageManager::insert(Page &page, const uint8_t *data, uint16_t size) {
  auto &h = page.layout.header;

  uint16_t needed = size + SLOT_SIZE;

  if (freeSpace(page) < needed) {
    throw std::runtime_error("page full");
  }

  h.upper -= size;
  uint16_t writeOffset = h.upper;

  std::memcpy(page.data.data() + writeOffset, data, size);

  SlotId slotId = h.slotCount++;

  // TODO possible resize efficiency problems
  if (page.layout.slots.size() <= slotId) {
    page.layout.slots.resize(slotId + 1);
  }

  h.lower += SLOT_SIZE;

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

  std::vector<uint8_t> old = page.data;

  uint16_t writeOffset = page.data.size();

  for (Slot &s : page.layout.slots) {
    if (s.state == SLOT_DELETED)
      continue;

    writeOffset -= s.size;

    std::memcpy(page.data.data() + writeOffset, old.data() + s.offset, s.size);

    s.offset = writeOffset;
  }

  h.upper = writeOffset;

  page.dirty = true;
}
