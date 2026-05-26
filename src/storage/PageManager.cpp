#include "PageManager.h"

#include <cstring>
#include <stdexcept>

using namespace vault::storage;

SlotId PageManager::insert(Page &page, const uint8_t *data, uint16_t size) {
  auto &hdr = page.layout.header;

  uint16_t slotId = hdr.slotCount++;
  uint16_t pos = hdr.freeStart;

  if (pos + size > page.data.size()) {
    throw std::runtime_error("page full");
  }

  std::memcpy(page.data.data() + pos, data, size);

  if (page.layout.slots.size() <= slotId) {
    page.layout.slots.resize(slotId + 1);
  }

  page.layout.slots[slotId] =
      Slot{.offset = pos, .size = size, .deleted = false};

  hdr.freeStart += size;
  page.dirty = true;

  return slotId;
}

std::vector<uint8_t> PageManager::read(Page &page, SlotId slotId) {
  if (slotId >= page.layout.slots.size()) {
    throw std::runtime_error("invalid slot");
  }

  const Slot &s = page.layout.slots[slotId];

  if (s.deleted) {
    throw std::runtime_error("slot deleted");
  }

  std::vector<uint8_t> out(s.size);
  std::memcpy(out.data(), page.data.data() + s.offset, s.size);

  return out;
}

std::optional<SlotId> PageManager::update(Page &page, SlotId slotId,
                                          const uint8_t *data, uint16_t size) {
  if (slotId >= page.layout.slots.size()) {
    throw std::runtime_error("invalid slot");
  }

  Slot &s = page.layout.slots[slotId];

  if (s.deleted) {
    throw std::runtime_error("slot deleted");
  }

  // If fits -> overwrite in place
  if (size <= s.size) {
    std::memcpy(page.data.data() + s.offset, data, size);

    s.size = size;
    page.dirty = true;

    return std::nullopt;
  }

  // If it's too large -> relocate
  s.deleted = true;

  SlotId newSlot = insert(page, data, size);

  page.dirty = true;
  return newSlot;
}

void PageManager::remove(Page &page, SlotId slotId) {
  if (slotId >= page.layout.slots.size()) {
    throw std::runtime_error("invalid slot");
  }

  page.layout.slots[slotId].deleted = true;
  page.dirty = true;
}

// Sliding compaction
void PageManager::compact(Page &page) {
  auto &slots = page.layout.slots;
  auto &hdr = page.layout.header;

  std::vector<uint8_t> old = page.data;

  uint16_t writePos = 0;

  for (Slot &s : slots) {
    if (s.deleted) {
      continue;
    }

    if (writePos + s.size > page.data.size()) {
      throw std::runtime_error("compaction overflow");
    }

    std::memcpy(page.data.data() + writePos, old.data() + s.offset, s.size);

    s.offset = writePos;
    writePos += s.size;
  }

  hdr.freeStart = writePos;

  page.dirty = true;
}
