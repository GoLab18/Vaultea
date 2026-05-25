#include "Pager.h"

#include <stdexcept>

Pager::Pager(StorageEngine &storage, uint32_t pageSize, size_t maxPages)
    : storage(storage), pageSize(pageSize), maxPages(maxPages) {
  nextPageId = storage.fileSize() / pageSize;
}

Page &Pager::getPage(PageId id) {
  auto it = cache.find(id);

  if (it == cache.end()) {
    if (cache.size() >= maxPages) {
      evictOne();
    }

    loadPage(id);
    it = cache.find(id);
  }

  it->second.pinCount++;
  touch(id);

  return it->second;
}

void Pager::unpin(PageId id) {
  auto it = cache.find(id);
  if (it == cache.end())
    return;

  if (it->second.pinCount > 0) {
    it->second.pinCount--;
  }
}

void Pager::markDirty(PageId id) {
  auto it = cache.find(id);
  if (it == cache.end()) {
    throw std::runtime_error("markDirty: page not cached");
  }

  it->second.dirty = true;
}

void Pager::touch(PageId id) {
  auto it = lruMap.find(id);

  if (it != lruMap.end()) {
    lru.erase(it->second);
    lruMap.erase(it);
  }

  lru.push_front(id);
  lruMap[id] = lru.begin();
}

void Pager::evictOne() {
  while (!lru.empty()) {
    PageId victimId = lru.back();
    lru.pop_back();

    auto it = cache.find(victimId);
    if (it == cache.end()) {
      lruMap.erase(victimId);
      continue;
    }

    Page &page = it->second;

    if (page.pinCount > 0) {
      lru.push_front(victimId);
      lruMap[victimId] = lru.begin();
      continue;
    }

    if (page.dirty) {
      writePageToDisk(page);
    }

    cache.erase(it);
    lruMap.erase(victimId);
    return;
  }

  throw std::runtime_error("evictOne: no evictable pages");
}

void Pager::loadPage(PageId id) {
  Page page;
  page.id = id;
  page.data.resize(pageSize, 0);
  page.dirty = false;
  page.pinCount = 0;

  uint64_t offset = pageOffset(id);
  uint64_t fileSize = storage.fileSize();

  if (offset < fileSize) {
    auto raw = storage.read(offset, pageSize);
    page.data = std::move(raw);
  }

  cache[id] = std::move(page);
}

void Pager::writePageToDisk(Page &page) {
  storage.write(pageOffset(page.id), page.data.data(), pageSize);
  page.dirty = false;
}

Pager::PageId Pager::allocatePage() {
  if (!freePages.empty()) {
    PageId id = freePages.back();
    freePages.pop_back();
    return id;
  }

  return nextPageId++;
}

void Pager::freePage(PageId id) {
  auto it = cache.find(id);

  if (it != cache.end()) {
    if (it->second.pinCount > 0) {
      throw std::runtime_error("freePage: page is pinned");
    }

    if (it->second.dirty) {
      writePageToDisk(it->second);
    }

    cache.erase(it);
  }

  auto lm = lruMap.find(id);
  if (lm != lruMap.end()) {
    lru.erase(lm->second);
    lruMap.erase(lm);
  }

  freePages.push_back(id);
}

void Pager::flush() {
  for (auto &[id, page] : cache) {
    if (page.dirty) {
      writePageToDisk(page);
    }
  }

  storage.flush();
}
