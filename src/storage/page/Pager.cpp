#include "Pager.h"
#include "Constants.h"
#include "PageLayout.h"
#include "pipeline/Serialization.h"

#include <cstddef>
#include <memory>
#include <stdexcept>

Pager::Pager(StorageEngine &storage, uint32_t pageSize,
             uint64_t pageRegionOffset, PageId freelistRootPage)
    : storage(storage),
      allocator(pageSize, pageRegionOffset, loadFreelist(freelistRootPage),
                (storage.fileSize() <= pageRegionOffset)
                    ? 0
                    : (storage.fileSize() - pageRegionOffset + pageSize - 1) /
                          pageSize),
      pageSize(pageSize), freelistRootPage(freelistRootPage) {}

std::deque<PageId> Pager::loadFreelist(PageId root) {
  std::deque<PageId> freePages;

  PageId current = root;

  while (current != INVALID_PAGE) {
    Page &page = getPage(current);

    auto &layout = page.layout->as<FreelistLayout>();

    freePages.insert(freePages.end(), layout.freePages.begin(),
                     layout.freePages.end());

    auto next = layout.header.nextPage;

    if (next == INVALID_PAGE) {
        freelistTailPage = current;
    }

    current = next;

    unpin(page.id);
  }

  return freePages;
}

void Pager::persistFreelist() {
  auto &freePages = allocator.getFreePages();

  const bool wasAllocated = allocator.wasAllocatedSinceLastFlush();
  const size_t newlyFreedCount = allocator.getNewlyFreeCount();

  if (!wasAllocated && newlyFreedCount == 0) {
    return;
  }

  const uint16_t maxPerPage = FreelistLayout::maxFreePages(pageSize);

  PageId firstPage = INVALID_PAGE;
  PageId prev = INVALID_PAGE;

  auto flushAll = [&]() {
    if (freePages.empty()) {
      PageId current = freelistRootPage;

      while (current != INVALID_PAGE) {
        Page &page = getPage(current);
        PageId next = page.layout->header.nextPage;

        freePage(current);
        current = next;
      }

      freelistRootPage = INVALID_PAGE;
      freelistTailPage = INVALID_PAGE;
      return;
    }

    size_t freeIndex = 0;

    PageId current = freelistRootPage;
    prev = INVALID_PAGE;
    firstPage = INVALID_PAGE;

    std::vector<PageId> obsoletePages;

    // Reusing existing freelist pages

    while (current != INVALID_PAGE) {
      Page &page = getPage(current);
      auto &layout = page.layout->as<FreelistLayout>();

      PageId next = layout.header.nextPage;

      layout.freePages.clear();

      size_t count = 0;
      while (count < maxPerPage && freeIndex < freePages.size()) {
        layout.freePages.push_back(freePages[freeIndex]);
        ++freeIndex;
        ++count;
      }

      layout.header.prevPage = prev;
      layout.header.nextPage = INVALID_PAGE;

      page.dirty = true;

      if (prev != INVALID_PAGE) {
        Page &prevPage = getPage(prev);
        prevPage.layout->header.nextPage = current;
        prevPage.dirty = true;
      }

      if (firstPage == INVALID_PAGE) {
        firstPage = current;
      }

      prev = current;

      // No more free page ids left -> freelist pages reminder becomes obsolete

      if (freeIndex >= freePages.size()) {
        current = next;

        while (current != INVALID_PAGE) {
          Page &obsolete = getPage(current);
          PageId nextObsolete = obsolete.layout->header.nextPage;

          obsoletePages.push_back(current);
          current = nextObsolete;
        }

        break;
      }

      current = next;
    }

    // More freelist pages needed

    while (freeIndex < freePages.size()) {
      PageId pageId = allocatePage(PageType::Freelist);
      Page &page = getPage(pageId);

      auto &layout = page.layout->as<FreelistLayout>();
      layout.freePages.clear();

      size_t count = 0;
      while (count < maxPerPage && freeIndex < freePages.size()) {
        layout.freePages.push_back(freePages[freeIndex]);

        freeIndex++;
        count++;
      }

      layout.header.prevPage = prev;
      layout.header.nextPage = INVALID_PAGE;

      page.dirty = true;

      if (prev != INVALID_PAGE) {
        Page &prevPage = getPage(prev);
        prevPage.layout->header.nextPage = pageId;
        prevPage.dirty = true;
      }

      if (firstPage == INVALID_PAGE) {
        firstPage = pageId;
      }

      prev = pageId;
    }

    freelistRootPage = firstPage;
    freelistTailPage = prev;
    if (firstPage == INVALID_PAGE) {
        freelistTailPage = INVALID_PAGE;
    }

    for (PageId id : obsoletePages) {
      freePage(id);
    }
  };

  auto flushAppendOnly = [&]() {
    if (freelistRootPage == INVALID_PAGE) {
      flushAll();
      return;
    }

    PageId lastPageId = freelistTailPage;
    if (lastPageId == INVALID_PAGE) {
        flushAll();
        return;
    }

    Page &lastPage = getPage(lastPageId);
    auto *layout = &lastPage.layout->as<FreelistLayout>();

    size_t start = freePages.size() - newlyFreedCount;

    for (size_t i = start; i < freePages.size(); ++i) {
      if (layout->freePages.size() == maxPerPage) {
        PageId newPageId = allocatePage(PageType::Freelist);
        Page &newPage = getPage(newPageId);

        auto &newLayout = newPage.layout->as<FreelistLayout>();
        newLayout.freePages.clear();

        newLayout.header.prevPage = lastPageId;
        newLayout.header.nextPage = INVALID_PAGE;

        newPage.dirty = true;

        lastPage.layout->header.nextPage = newPageId;
        lastPage.dirty = true;

        freelistTailPage = newPageId;

        lastPageId = newPageId;
        layout = &newLayout;
      }

      layout->freePages.push_back(freePages[i]);
    }
  };

  if (wasAllocated) {
    flushAll();
  } else {
    flushAppendOnly();
  }

  allocator.clearState();
}

Page &Pager::getPage(PageId id) {
  auto it = cache.find(id);

  if (it == cache.end()) {
    if (cache.size() >= MAX_PAGE_CACHE_SIZE) {
      evictOne();
    }

    loadPage(id);
    it = cache.find(id);
  }

  touch(id);

  return it->second;
}

void Pager::pin(PageId id) {
  Page &page = getPage(id);
  page.pinCount++;
}

void Pager::unpin(PageId id) {
  auto it = cache.find(id);
  if (it == cache.end())
    return;

  if (it->second.pinCount > 0) {
    it->second.pinCount--;
  }
}

PageId Pager::allocatePage(PageType type) {
  PageId id = allocator.allocate();

  auto it = cache.find(id);

  if (it != cache.end()) {
    Page &page = it->second;

    page = createPage(id, type);

    touch(id);

    return id;
  }

  if (cache.size() >= MAX_PAGE_CACHE_SIZE) {
    evictOne();
  }

  cache[id] = createPage(id, type);

  touch(id);

  return id;
}

Page Pager::createPage(PageId id, PageType type) {
  Page page;

  page.id = id;
  page.data.resize(pageSize);

  PageHeader header;
  header.pageId = id;
  header.type = type;

  auto layoutType = toLayoutType(header.type);

  switch (layoutType) {
  case LayoutType::Slotted:
    page.layout = std::make_unique<SlottedLayout>(header, pageSize);
    break;

  case LayoutType::Freelist:
    page.layout = std::make_unique<FreelistLayout>(header);
    break;

  case LayoutType::Free:
    page.layout = std::make_unique<FreeLayout>(header);
    break;
  }

  page.dirty = true;

  return page;
}

void Pager::freePage(PageId id) {
  Page &page = getPage(id);

  if (page.pinCount > 1) {
    throw std::runtime_error("page pinned");
  }

  PageHeader header;
  header.pageId = id;

  page.layout = std::make_unique<FreeLayout>(header);

  std::fill(page.data.begin(), page.data.end(), 0);

  page.dirty = true;

  allocator.free(id);

  unpin(id);
}

void Pager::touch(PageId id) {
  auto it = cache.find(id);
  if (it == cache.end())
    return;

  if (it->second.pinCount > 0) {
    return;
  }

  auto lruIt = lruMap.find(id);
  if (lruIt != lruMap.end()) {
    lru.erase(lruIt->second);
    lruMap.erase(lruIt);
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
      continue;
    }

    if (page.dirty) {
      writePageToDisk(page);
    }

    cache.erase(it);
    lruMap.erase(victimId);
    return;
  }

  throw std::runtime_error("no evictable pages");
}

void Pager::loadPage(PageId id) {
  Page page;
  page.id = id;
  page.data.resize(pageSize);

  uint64_t offset = allocator.pageOffset(id);

  if (offset + pageSize > storage.fileSize()) {
    throw std::runtime_error("page not found on disk");
  }

  page.data = storage.read(offset, pageSize);

  PageHeader header = Serialization::deserializePageHeader(page.data);

  auto layoutType = toLayoutType(header.type);

  switch (layoutType) {
  case LayoutType::Slotted: {
    SlottedLayout layout =
        Serialization::deserializeSlottedLayout(header, pageSize, page.data);

    page.layout = std::make_unique<SlottedLayout>(std::move(layout));
    break;
  }

  case LayoutType::Freelist: {
    FreelistLayout layout =
        Serialization::deserializeFreelistLayout(header, page.data);

    page.layout = std::make_unique<FreelistLayout>(std::move(layout));
    break;
  }

  case LayoutType::Free: {
    FreeLayout layout = Serialization::deserializeFreeLayout(header);

    page.layout = std::make_unique<FreeLayout>(std::move(layout));
    break;
  }
  }

  cache[id] = std::move(page);
}

void Pager::writePageToDisk(Page &page) {
  RawBytes headerBytes =
      Serialization::serializePageHeader(page.layout->header);

  std::copy(headerBytes.begin(), headerBytes.end(), page.data.begin());

  RawBytes layoutBytes;

  auto layoutType = toLayoutType(page.layout->header.type);

  switch (layoutType) {
  case LayoutType::Slotted:
    layoutBytes =
        Serialization::serializeSlottedLayout(page.layout->as<SlottedLayout>());
    break;

  case LayoutType::Freelist:
    layoutBytes = Serialization::serializeFreelistLayout(
        page.layout->as<FreelistLayout>());
    break;

  case LayoutType::Free:
    layoutBytes =
        Serialization::serializeFreeLayout(page.layout->as<FreeLayout>());
    break;
  }

  std::copy(layoutBytes.begin(), layoutBytes.end(),
            page.data.begin() + HEADER_SIZE);

  storage.write(allocator.pageOffset(page.id), page.data.data(), pageSize);

  page.dirty = false;
}

void Pager::flush() {
  persistFreelist();

  for (auto &[id, page] : cache) {
    if (page.dirty) {
      writePageToDisk(page);
    }
  }

  storage.flush();
}

PageId Pager::getFreelistRootPage() const { return freelistRootPage; }
