#pragma once

#include "Page.h"
#include "PageAllocator.h"
#include "storage/StorageEngine.h"

#include <deque>
#include <list>
#include <unordered_map>

using namespace vault::storage::page;

class Pager {
public:
  Pager(StorageEngine &storage, uint32_t pageSize, uint64_t pageRegionOffset,
        PageId freelistRootPage);

  std::deque<PageId> loadFreelist(PageId freelistRootPage);
  void persistFreelist();

  Page &getPage(PageId id);

  void pin(PageId id);
  void unpin(PageId id);

  PageId allocatePage(PageType type);

  void freePage(PageId id);

  void flush();

  PageId getFreelistRootPage() const;

private:
  StorageEngine &storage;
  PageAllocator allocator;

  uint32_t pageSize;

  PageId freelistRootPage;
  PageId freelistTailPage;

  std::unordered_map<PageId, Page> cache;

  std::list<PageId> lru;
  std::unordered_map<PageId, std::list<PageId>::iterator> lruMap;

  void touch(PageId id);
  void evictOne();

  void loadPage(PageId id);

  void writePageToDisk(Page &page);

  Page createPage(PageId id, PageType type);
};
