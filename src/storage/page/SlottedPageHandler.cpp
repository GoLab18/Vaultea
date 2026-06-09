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

  for (const auto &slot : layout.pageSlots) {
    if (slot.state == SlotState::SlotDeleted)
      return true;
  }
  return false;
}

std::optional<SlotId> SlottedPageHandler::findReusableSlot(const Page &page) {
  auto &layout = page.layout->as<SlottedLayout>();

  for (uint16_t i = 0; i < layout.pageSlots.size(); i++) {
    if (layout.pageSlots[i].state == SlotState::SlotDeleted)
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
    slotId = static_cast<SlotId>(layout.pageSlots.size());
    layout.pageSlots.emplace_back();
    layout.lower += SLOT_SIZE;
  }

  layout.pageSlots[slotId] = {
      .offset = writeOffset, .size = size, .state = SlotState::SlotUsed};

  return slotId;
}

RawBytes SlottedPageHandler::read(Page &page, SlotId slotId) {
  auto &layout = page.layout->as<SlottedLayout>();

  if (slotId >= layout.pageSlots.size())
    throw std::runtime_error("invalid slot");

  const Slot &s = layout.pageSlots[slotId];

  if (s.state == SlotState::SlotDeleted)
    throw std::runtime_error("slot deleted");

  RawBytes out(s.size);
  std::memcpy(out.data(), page.data.data() + s.offset, s.size);

  return out;
}

UpdateResult SlottedPageHandler::update(Page &page, SlotId slotId,
                                        const uint8_t *data, uint16_t size) {
  auto &layout = page.layout->as<SlottedLayout>();

  if (slotId >= layout.pageSlots.size())
    throw std::runtime_error("invalid slot");

  Slot &s = layout.pageSlots[slotId];

  if (s.state == SlotState::SlotDeleted)
    throw std::runtime_error("slot deleted");

  if (size <= s.size) {
    std::memcpy(page.data.data() + s.offset, data, size);
    s.size = size;

    return UpdateResult::UpdatedInPlace;
  }

  return UpdateResult::RequiresRelocation;
}

// Returns true if the page has been fully freed, otherwise false
bool SlottedPageHandler::remove(Page &page, SlotId slotId) {
  auto &layout = page.layout->as<SlottedLayout>();

  if (slotId >= layout.pageSlots.size())
    throw std::runtime_error("invalid slot");

  layout.pageSlots[slotId].state = SlotState::SlotDeleted;

  for (const auto &slot : layout.pageSlots) {
    if (slot.state == SlotState::SlotUsed)
      return false;
  }

  return true;
}

// Sliding compaction
void SlottedPageHandler::compact(Page &page) {
  auto &layout = page.layout->as<SlottedLayout>();

  uint8_t *data = page.data.data();

  std::vector<Slot *> active;
  active.reserve(layout.pageSlots.size());

  for (auto &slot : layout.pageSlots)
    if (slot.state == SlotState::SlotUsed)
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
}
