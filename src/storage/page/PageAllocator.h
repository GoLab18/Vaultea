#pragma once

#include "Constants.h"

#include <cstdint>
#include <unordered_set>

using namespace vault::storage::page;

class PageAllocator {
public:
  explicit PageAllocator(uint64_t pageSize, uint64_t pageRegionOffset,
                         std::unordered_set<PageId> freePages,
                         PageId nextPageId);

  PageId allocate();
  void free(PageId id);

  uint64_t pageOffset(PageId id) const;

private:
  uint64_t pageSize;
  uint64_t pageRegionOffset;

  PageId nextPageId;
  std::unordered_set<PageId> freePages;
};
