#pragma once

#include "Constants.h"
#include "Page.h"
#include "StorageEngine.h"

#include <cstdint>
#include <list>
#include <unordered_map>
#include <vector>

class Pager {
public:
  using PageId = vault::storage::PageId;

  Pager(StorageEngine &storage, uint32_t pageSize, size_t maxPages);

  Page &getPage(PageId id);

  void unpin(PageId id);
  void markDirty(PageId id);

  PageId allocatePage();
  void freePage(PageId id);

  void flush();

private:
  StorageEngine &storage;
  uint32_t pageSize;
  size_t maxPages;

  std::unordered_map<PageId, Page> cache;

  std::list<PageId> lru;
  std::unordered_map<PageId, std::list<PageId>::iterator> lruMap;

  std::vector<PageId> freePages;

  PageId nextPageId = 0;

private:
  inline uint64_t pageOffset(PageId id) const {
    return static_cast<uint64_t>(id) * pageSize;
  }

  void touch(PageId id);
  void evictOne();
  void loadPage(PageId id);
  void writePageToDisk(Page &page);
};
