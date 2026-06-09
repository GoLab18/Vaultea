#include "SlottedManager.h"
#include "storage/page/SlottedPageHandler.h"

SlottedManager::SlottedManager(Pager &pager, PageId rootPage, PageType pageType)
    : pager(pager), pageType(pageType), rootPage(rootPage) {
  if (rootPage == INVALID_PAGE) {
    this->rootPage = allocatePage();
    this->tailPage = this->rootPage;
  } else {
    loadFreeSpaceMap();
  }
}

PageId SlottedManager::getRootPage() const { return rootPage; }

PageId SlottedManager::allocatePage() {
  PageId id = pager.allocatePage(pageType);

  Page &page = pager.getPage(id);
  pager.pin(id);

  auto &layout = page.layout->as<SlottedLayout>();

  registerPage(id, layout.upper - layout.lower);

  page.dirty = true;

  pager.unpin(id);

  return id;
}

PageId SlottedManager::findPageWithSpace(uint16_t size) {
  auto it = freeSpaceMap.lower_bound(size);

  if (it != freeSpaceMap.end()) {
    return it->second;
  }

  PageId newPageId = allocatePage();

  if (tailPage != INVALID_PAGE) {
    Page &tail = pager.getPage(tailPage);
    pager.pin(tailPage);

    tail.layout->header.nextPage = newPageId;
    tail.dirty = true;

    Page &newPage = pager.getPage(newPageId);
    pager.pin(newPageId);

    newPage.layout->header.prevPage = tailPage;
    newPage.dirty = true;

    pager.unpin(newPageId);
    pager.unpin(tailPage);
  }

  tailPage = newPageId;

  return newPageId;
}

RecordRef SlottedManager::insertRecord(const RawBytes &bytes) {
  PageId pageId = findPageWithSpace(static_cast<uint16_t>(bytes.size()));

  Page &page = pager.getPage(pageId);
  pager.pin(pageId);

  auto &layout = page.layout->as<SlottedLayout>();

  SlotId slot = SlottedPageHandler::insert(page, bytes.data(),
                                           static_cast<uint16_t>(bytes.size()));

  FreeSpace after = layout.upper - layout.lower;
  updatePageSpace(pageId, after);

  page.dirty = true;

  pager.unpin(pageId);

  return {pageId, slot};
}

RawBytes SlottedManager::readRecord(const RecordRef &ref) {
  Page &page = pager.getPage(ref.pageId);
  pager.pin(ref.pageId);

  auto bytes = SlottedPageHandler::read(page, ref.slotId);

  pager.unpin(ref.pageId);

  return bytes;
}

std::optional<RecordRef> SlottedManager::updateRecord(const RecordRef &ref,
                                                      const RawBytes &bytes) {
  Page &page = pager.getPage(ref.pageId);
  pager.pin(ref.pageId);

  auto &layout = page.layout->as<SlottedLayout>();

  auto result = SlottedPageHandler::update(page, ref.slotId, bytes.data(),
                                           static_cast<uint16_t>(bytes.size()));

  switch (result) {
  case UpdateResult::UpdatedInPlace: {
    FreeSpace after = layout.upper - layout.lower;
    updatePageSpace(ref.pageId, after);

    page.dirty = true;

    pager.unpin(ref.pageId);

    return std::nullopt;
  }

  case UpdateResult::RequiresRelocation: {
    pager.unpin(ref.pageId);

    deleteRecord(ref);

    RecordRef newRef = insertRecord(bytes);

    return newRef;
  }
  }
}

void SlottedManager::deleteRecord(const RecordRef &ref) {
  Page &page = pager.getPage(ref.pageId);
  pager.pin(ref.pageId);

  auto &layout = page.layout->as<SlottedLayout>();

  bool becameEmpty = SlottedPageHandler::remove(page, ref.slotId);
  if (becameEmpty) {
    pager.unpin(ref.pageId);

    freeEmptyPage(ref.pageId);
    return;
  }

  FreeSpace after = layout.upper - layout.lower;
  updatePageSpace(ref.pageId, after);

  page.dirty = true;

  pager.unpin(ref.pageId);
}

void SlottedManager::loadFreeSpaceMap() {
  PageId current = rootPage;
  PageId prev = INVALID_PAGE;

  while (current != INVALID_PAGE) {
    Page &page = pager.getPage(current);
    pager.pin(current);

    auto &layout = page.layout->as<SlottedLayout>();

    FreeSpace fs = layout.upper - layout.lower;

    registerPage(current, fs);

    pager.unpin(current);

    prev = current;
    current = layout.header.nextPage;
  }

  tailPage = prev;
}

void SlottedManager::freeEmptyPage(PageId id) {
  if (id == rootPage)
    return;

  Page &page = pager.getPage(id);

  auto &header = page.layout->header;

  PageId prev = header.prevPage;
  PageId next = header.nextPage;

  if (prev != INVALID_PAGE) {
    Page &prevPage = pager.getPage(prev);

    prevPage.layout->header.nextPage = next;
    prevPage.dirty = true;
  }

  if (next != INVALID_PAGE) {
    Page &nextPage = pager.getPage(next);

    nextPage.layout->header.prevPage = prev;
    nextPage.dirty = true;
  }

  if (tailPage == id)
    tailPage = prev;

  unregisterPage(id);

  pager.freePage(id);
}

void SlottedManager::registerPage(PageId id, FreeSpace fs) {
  auto it = freeSpaceMap.emplace(fs, id);
  pagePositions[id] = it;
}

void SlottedManager::updatePageSpace(PageId id, FreeSpace newFs) {
  auto it = pagePositions.find(id);
  if (it != pagePositions.end()) {
    freeSpaceMap.erase(it->second);
  }

  registerPage(id, newFs);
}

void SlottedManager::unregisterPage(PageId id) {
  auto it = pagePositions.find(id);
  if (it != pagePositions.end()) {
    freeSpaceMap.erase(it->second);
    pagePositions.erase(it);
  }
}
