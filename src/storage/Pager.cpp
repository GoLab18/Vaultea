#include "Pager.h"

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

  if (it == cache.end()) {
    return;
  }

  if (it->second.pinCount > 0) {
    it->second.pinCount--;
  }
}

void Pager::markDirty(PageId id) {
  auto it = cache.find(id);

  if (it == cache.end()) {
    throw std::runtime_error("Page not cached");
  }

  it->second.dirty = true;
}

void Pager::touch(PageId id) {
  auto it = lruMap.find(id);

  if (it != lruMap.end()) {
    lru.erase(it->second);
  }

  lru.push_front(id);

  lruMap[id] = lru.begin();
}

void Pager::evictOne() {

  for (auto it = lru.rbegin(); it != lru.rend(); ++it) {
    PageId victimId = *it;

    auto pageIt = cache.find(victimId);

    if (pageIt == cache.end()) {
      continue;
    }

    Page &page = pageIt->second;

    if (page.pinCount > 0) {
      continue;
    }

    if (page.dirty) {
      writePageToDisk(page);
    }

    lru.erase(lruMap[victimId]);

    lruMap.erase(victimId);

    cache.erase(victimId);

    return;
  }

  throw std::runtime_error("No evictable pages available");
}

void Pager::loadPage(PageId id) {

  auto raw = storage.read(pageOffset(id), pageSize);

  Page page;

  page.id = id;

  page.data = std::move(raw);

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
      throw std::runtime_error("Cannot free pinned page");
    }

    lru.erase(lruMap[id]);

    lruMap.erase(id);

    cache.erase(it);
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
