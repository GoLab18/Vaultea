#pragma once

#include <cstdint>
#include <vector>

struct Slot {
  uint16_t offset;
  uint16_t size;
  bool deleted;
};

struct PageHeader {
  uint16_t slotCount = 0;
  uint16_t freeStart;
  uint16_t freeSpace;
};

struct PageLayout {
  PageHeader header;
  std::vector<Slot> slots;
};
