#include "SlottedPageHandler.h"
#include "Constants.h"

#include <algorithm>
#include <cstring>

uint16_t SlottedPageHandler::freeSpace(const Page &page) {
  auto &layout = page.layout->as<SlottedLayout>();
  return layout.upper - layout.lower;
}

bool SlottedPageHandler::hasDeletedSlots(const Page &page) {
  auto &layout = page.layout->as<SlottedLayout>();

  for (const auto &slot : layout.slots) {
    if (slot.state == SLOT_DELETED)
      return true;
  }
  return false;
}

std::optional<SlotId> SlottedPageHandler::findReusableSlot(const Page &page) {
  auto &layout = page.layout->as<SlottedLayout>();

  for (uint16_t i = 0; i < layout.slots.size(); i++) {
    if (layout.slots[i].state == SLOT_DELETED)
      return i;
  }

  return std::nullopt;
}

SlotId SlottedPageHandler::insert(Page &page, const uint8_t *data,
                                  uint16_t size) {
  auto &layout = page.layout->as<SlottedLayout>();

  auto reusable = findReusableSlot(page);
  bool needsNewSlot = !reusable.has_value();
  uint16_t needed = size + (needsNewSlot ? SLOT_SIZE : 0);

  if (freeSpace(page) < needed && hasDeletedSlots(page)) {
    compact(page);
  }

  if (freeSpace(page) < needed) {
    throw std::runtime_error("page full");
  }

  layout.upper -= size;
  uint16_t writeOffset = layout.upper;

  std::memcpy(page.data.data() + writeOffset, data, size);

  SlotId slotId;

  if (reusable) {
    slotId = *reusable;
  } else {
    slotId = static_cast<SlotId>(layout.slots.size());
    layout.slots.emplace_back();
    layout.lower += SLOT_SIZE;
  }

  layout.slots[slotId] = {
      .offset = writeOffset, .size = size, .state = SLOT_USED};

  page.dirty = true;
  return slotId;
}

RawBytes SlottedPageHandler::read(Page &page, SlotId slotId) {
  auto &layout = page.layout->as<SlottedLayout>();

  if (slotId >= layout.slots.size())
    throw std::runtime_error("invalid slot");

  const Slot &s = layout.slots[slotId];

  if (s.state == SLOT_DELETED)
    throw std::runtime_error("slot deleted");

  RawBytes out(s.size);
  std::memcpy(out.data(), page.data.data() + s.offset, s.size);

  return out;
}

std::optional<SlotId> SlottedPageHandler::update(Page &page, SlotId slotId,
                                                 const uint8_t *data,
                                                 uint16_t size) {
  auto &layout = page.layout->as<SlottedLayout>();

  if (slotId >= layout.slots.size())
    throw std::runtime_error("invalid slot");

  Slot &s = layout.slots[slotId];

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

void SlottedPageHandler::remove(Page &page, SlotId slotId) {
  auto &layout = page.layout->as<SlottedLayout>();

  if (slotId >= layout.slots.size())
    throw std::runtime_error("invalid slot");

  layout.slots[slotId].state = SLOT_DELETED;
  page.dirty = true;
}

// Sliding compaction
void SlottedPageHandler::compact(Page &page) {
  auto &layout = page.layout->as<SlottedLayout>();

  uint8_t *data = page.data.data();

  std::vector<Slot *> active;
  active.reserve(layout.slots.size());

  for (auto &slot : layout.slots)
    if (slot.state == SLOT_USED)
      active.push_back(&slot);

  std::sort(active.begin(), active.end(),
            [](auto *a, auto *b) { return a->offset > b->offset; });

  uint16_t writeOffset = static_cast<uint16_t>(page.data.size());

  for (auto *slot : active) {
    writeOffset -= slot->size;
    std::memmove(data + writeOffset, data + slot->offset, slot->size);
    slot->offset = writeOffset;
  }

  layout.upper = writeOffset;
  page.dirty = true;
}
