#include "Pager.h"
#include "PageLayout.h"
#include "pipeline/Serialization.h"

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
      pageSize(pageSize) {}

std::unordered_set<PageId> Pager::loadFreelist(PageId freelistRootPage) {
  std::unordered_set<PageId> freePages;

  PageId current = freelistRootPage;

  while (current != INVALID_PAGE) {
    Page &page = getPage(current);

    auto &layout = page.layout->as<FreelistLayout>();

    freePages.reserve(freePages.size() + layout.freePages.size());
    freePages.insert(layout.freePages.begin(), layout.freePages.end());

    unpin(current);
    current = layout.header.nextPage;
  }

  return freePages;
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

  // TODO it adds a pin and touches (?) -> something ain't right
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

PageId Pager::allocatePage(PageType type) {
  PageId id = allocator.allocate();

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
  header.type = PageType::Free;
  header.nextPage = INVALID_PAGE;

  page.layout = std::make_unique<FreeLayout>(header);

  // TODO might be redundant considering encryption
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

  // TODO shouldn't it throw otherwise (?)
  if (offset < storage.fileSize()) {
    page.data = storage.read(offset, pageSize);
  }

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
  for (auto &[id, page] : cache) {
    if (page.dirty) {
      writePageToDisk(page);
    }
  }

  storage.flush();
}
