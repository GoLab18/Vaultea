#pragma once

#include <cstdint>
#include <vector>

enum SlotState : uint8_t { SLOT_USED = 0, SLOT_DELETED = 1 };

struct Slot {
  uint16_t offset;
  uint16_t size;
  SlotState state;
};

struct PageHeader {
  uint16_t slotCount;

  uint16_t lower;
  uint16_t upper;
};

struct PageLayout {
  PageHeader header;
  std::vector<Slot> slots;
};
