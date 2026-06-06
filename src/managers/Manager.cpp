#include "Manager.h"

Manager::Manager(Pager &pager, PageId rootPage)
    : pager(pager), rootPage(rootPage), tailPage(INVALID_PAGE) {
  loadFreeSpaceMap();
}

void Manager::insertPage(PageId id, FreeSpace fs) {
  auto it = freeSpaceMap.emplace(fs, id);
  pagePositions[id] = it;
}

void Manager::updatePage(PageId id, FreeSpace oldFs, FreeSpace newFs) {
  auto it = pagePositions.find(id);
  if (it != pagePositions.end()) {
    freeSpaceMap.erase(it->second);
  }
  insertPage(id, newFs);
}

void Manager::removePage(PageId id) {
  auto it = pagePositions.find(id);
  if (it != pagePositions.end()) {
    freeSpaceMap.erase(it->second);
    pagePositions.erase(it);
  }
}

void Manager::loadFreeSpaceMap() {
  PageId current = rootPage;
  PageId prev = INVALID_PAGE;

  while (current != INVALID_PAGE) {
    Page &page = pager.getPage(current);
    auto &layout = page.layout->as<SlottedLayout>();

    FreeSpace fs = layout.upper - layout.lower;

    insertPage(current, fs);

    pager.unpin(current);

    prev = current;
    current = layout.header.nextPage;
  }

  tailPage = prev;
}
