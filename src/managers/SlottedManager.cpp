#include "SlottedManager.h"
#include "storage/page/SlottedPageHandler.h"

SlottedManager::SlottedManager(Pager &pager, PageId rootPage, PageType pageType)
    : pager(pager), pageType(pageType), rootPage(rootPage) {
  loadFreeSpaceMap();
}

PageId SlottedManager::allocatePage() {
  PageId id = pager.allocatePage(pageType);

  Page &page = pager.getPage(id);

  auto &layout = page.layout->as<SlottedLayout>();

  registerPage(id, layout.upper - layout.lower);

  pager.unpin(id);

  return id;
}

PageId SlottedManager::findPageWithSpace(uint16_t size) {
  auto it = freeSpaceMap.lower_bound(size);

  if (it == freeSpaceMap.end()) {
    return allocatePage();
  }

  return it->second;
}

RecordRef SlottedManager::insertRecord(const RawBytes &bytes) {
  PageId pageId = findPageWithSpace(static_cast<uint16_t>(bytes.size()));

  Page &page = pager.getPage(pageId);

  auto &layout = page.layout->as<SlottedLayout>();

  SlotId slot = SlottedPageHandler::insert(page, bytes.data(),
                                           static_cast<uint16_t>(bytes.size()));

  FreeSpace after = layout.upper - layout.lower;

  updatePageSpace(pageId, after);

  pager.unpin(pageId);

  return {pageId, slot};
}

RawBytes SlottedManager::readRecord(const RecordRef &ref) {
  Page &page = pager.getPage(ref.pageId);

  auto bytes = SlottedPageHandler::read(page, ref.slotId);

  pager.unpin(ref.pageId);

  return bytes;
}

std::optional<RecordRef> SlottedManager::updateRecord(const RecordRef &ref,
                                                      const RawBytes &bytes) {
  Page &page = pager.getPage(ref.pageId);

  auto &layout = page.layout->as<SlottedLayout>();

  auto moved = SlottedPageHandler::update(page, ref.slotId, bytes.data(),
                                          static_cast<uint16_t>(bytes.size()));

  FreeSpace after = layout.upper - layout.lower;

  updatePageSpace(ref.pageId, after);

  pager.unpin(ref.pageId);

  if (!moved)
    return std::nullopt;

  return RecordRef{ref.pageId, *moved};
}

void SlottedManager::deleteRecord(const RecordRef &ref) {
  Page &page = pager.getPage(ref.pageId);

  auto &layout = page.layout->as<SlottedLayout>();

  SlottedPageHandler::remove(page, ref.slotId);

  FreeSpace after = layout.upper - layout.lower;

  updatePageSpace(ref.pageId, after);

  pager.unpin(ref.pageId);
}

void SlottedManager::loadFreeSpaceMap() {
  PageId current = rootPage;
  PageId prev = INVALID_PAGE;

  while (current != INVALID_PAGE) {
    Page &page = pager.getPage(current);
    auto &layout = page.layout->as<SlottedLayout>();

    FreeSpace fs = layout.upper - layout.lower;

    registerPage(current, fs);

    pager.unpin(current);

    prev = current;
    current = layout.header.nextPage;
  }

  tailPage = prev;
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
