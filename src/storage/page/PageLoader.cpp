#include "PageLoader.h"
#include "Pager.h"
#include "models/IndexEntry.h"
#include "pipeline/Serialization.h"
#include "storage/BTreeIndex.h"

std::vector<PageId> PageLoader::loadFreelist(PageId rootFreelistPage,
                                             Pager &pager) {
  std::vector<PageId> freePages;

  PageId current = rootFreelistPage;

  while (current != INVALID_PAGE) {
    Page &page = pager.getPage(current);

    auto &layout = page.layout->as<FreelistLayout>();

    freePages.insert(freePages.end(), layout.freePages.begin(), layout.freePages.end());

    current = layout.header.nextPage;

    pager.unpin(current);
  }

  return freePages;
}

void PageLoader::loadIndex(PageId rootIndexPage, BTreeIndex &index,
                           Pager &pager) {
  std::vector<IndexEntry> allEntries;

  PageId current = rootIndexPage;

  while (current != INVALID_PAGE) {
    Page &page = pager.getPage(current);

    auto &layout = page.layout->as<SlottedLayout>();

    auto entries = Serialization::deserializeIndex(page.data);
    allEntries.insert(allEntries.end(), entries.begin(), entries.end());

    current = layout.header.nextPage;

    pager.unpin(page.id);
  }

  index.rebuild(allEntries);
}
