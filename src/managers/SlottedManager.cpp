#include "SlottedManager.h"
#include "storage/page/SlottedPageHandler.h"

SlottedManager::SlottedManager(Pager &pager, PageId rootPage, PageType pageType)
    : Manager(pager, rootPage), pageType(pageType) {}

PageId SlottedManager::allocatePage() {
  PageId id = pager.allocatePage(pageType);

  Page &page = pager.getPage(id);

  auto &layout = page.layout->as<SlottedLayout>();

  insertPage(id, layout.upper - layout.lower);

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

  FreeSpace before = layout.upper - layout.lower;

  SlotId slot = SlottedPageHandler::insert(page, bytes.data(),
                                           static_cast<uint16_t>(bytes.size()));

  FreeSpace after = layout.upper - layout.lower;

  updatePage(pageId, before, after);

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

  FreeSpace before = layout.upper - layout.lower;

  auto moved = SlottedPageHandler::update(page, ref.slotId, bytes.data(),
                                          static_cast<uint16_t>(bytes.size()));

  FreeSpace after = layout.upper - layout.lower;

  updatePage(ref.pageId, before, after);

  pager.unpin(ref.pageId);

  if (!moved)
    return std::nullopt;

  return RecordRef{ref.pageId, *moved};
}

void SlottedManager::deleteRecord(const RecordRef &ref) {
  Page &page = pager.getPage(ref.pageId);

  auto &layout = page.layout->as<SlottedLayout>();

  FreeSpace before = layout.upper - layout.lower;

  SlottedPageHandler::remove(page, ref.slotId);

  FreeSpace after = layout.upper - layout.lower;

  updatePage(ref.pageId, before, after);

  pager.unpin(ref.pageId);
}
