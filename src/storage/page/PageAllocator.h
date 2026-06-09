#pragma once

#include "Constants.h"

#include <cstdint>
#include <cstddef>
#include <deque>

using namespace vault::storage::page;

class PageAllocator {
public:
  explicit PageAllocator(uint64_t pageSize, uint64_t pageRegionOffset,
                         std::deque<PageId> freePages, PageId nextPageId);

  PageId allocate();
  void free(PageId id);

  uint64_t pageOffset(PageId id) const;

  const std::deque<PageId> &getFreePages() const;

  bool wasAllocatedSinceLastFlush() const;
  size_t getNewlyFreeCount() const;
  void clearState();

private:
  uint64_t pageSize;
  uint64_t pageRegionOffset;

  PageId nextPageId;

  std::deque<PageId> freePages;

  bool allocatedDirty = false;
  size_t newlyFreedCount = false;
};
