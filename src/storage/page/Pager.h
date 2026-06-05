#pragma once

#include "Page.h"
#include "PageAllocator.h"
#include "storage/StorageEngine.h"

#include <list>
#include <unordered_map>

using namespace vault::storage::page;

class Pager {
public:
  Pager(StorageEngine &storage, uint32_t pageSize, uint64_t pageRegionOffset,
        size_t maxPages, std::vector<PageId> freePages);

  Page &getPage(PageId id);

  void unpin(PageId id);

  PageId allocatePage(PageType type);

  void freePage(PageId id);

  void flush();

private:
  StorageEngine &storage;
  PageAllocator allocator;

  uint32_t pageSize;
  size_t maxPages;

  std::unordered_map<PageId, Page> cache;

  std::list<PageId> lru;
  std::unordered_map<PageId, std::list<PageId>::iterator> lruMap;

  void touch(PageId id);
  void evictOne();

  void loadPage(PageId id);

  void writePageToDisk(Page &page);

  Page createPage(PageId id, PageType type);
};
