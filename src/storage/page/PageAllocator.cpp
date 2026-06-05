#include "PageAllocator.h"

#include <stdexcept>

PageAllocator::PageAllocator(uint64_t pageSize, uint64_t pageRegionOffset,
                             std::unordered_set<PageId> freePages,
                             PageId nextPageId)
    : pageSize(pageSize), pageRegionOffset(pageRegionOffset),
      nextPageId(nextPageId), freePages(std::move(freePages)) {}

PageId PageAllocator::allocate() {
  if (!freePages.empty()) {
    auto it = freePages.begin();
    PageId id = *it;
    freePages.erase(it);
    return id;
  }

  return nextPageId++;
}

void PageAllocator::free(PageId id) {
  if (id >= nextPageId) {
    throw std::runtime_error("invalid free page id");
  }

  freePages.insert(id);
}

uint64_t PageAllocator::pageOffset(PageId id) const {
  return pageRegionOffset + id * pageSize;
}
