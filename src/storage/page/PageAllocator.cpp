#include "PageAllocator.h"

#include <stdexcept>

PageAllocator::PageAllocator(uint64_t pageSize, uint64_t pageRegionOffset,
                             std::deque<PageId> freePages, PageId nextPageId)
    : pageSize(pageSize), pageRegionOffset(pageRegionOffset),
      nextPageId(nextPageId), freePages(std::move(freePages)) {}

PageId PageAllocator::allocate() {
  PageId id;

  if (!freePages.empty()) {
    id = freePages.front();
    freePages.pop_front();
  } else {
    id = nextPageId++;
  }

  allocatedDirty = true;
  return id;
}

void PageAllocator::free(PageId id) {
  if (id >= nextPageId) {
    throw std::runtime_error("invalid free page id");
  }

  freePages.push_back(id);
  newlyFreedCount++;
}

uint64_t PageAllocator::pageOffset(PageId id) const {
  return pageRegionOffset + id * pageSize;
}

const std::deque<PageId> &PageAllocator::getFreePages() const {
  return freePages;
}

bool PageAllocator::wasAllocatedSinceLastFlush() const {
  return allocatedDirty;
}

size_t PageAllocator::getNewlyFreeCount() const { return newlyFreedCount; }

void PageAllocator::clearState() {
  allocatedDirty = false;
  newlyFreedCount = 0;
}
