#include "VaultLoader.h"
#include "models/IndexEntry.h"
#include "pipeline/Serialization.h"

#include <utility>

std::vector<PageId> VaultLoader::loadFreelist(PageId rootFreelistPage,
                                              Pager &pager) {
  std::vector<PageId> freePages;

  PageId current = rootFreelistPage;

  while (current != INVALID_PAGE) {
    Page &page = pager.getPage(current);

    auto &layout = page.layout->as<FreelistLayout>();

    freePages.reserve(freePages.size() + layout.freePages.size());
    freePages.insert(freePages.end(), layout.freePages.begin(),
                     layout.freePages.end());

    pager.unpin(current);
    current = layout.header.nextPage;
  }

  return freePages;
}

void VaultLoader::loadIndex(PageId rootIndexPage, BTreeIndex &index,
                            Pager &pager) {
  std::vector<LoadedIndexEntry> allEntries;

  PageId current = rootIndexPage;

  while (current != INVALID_PAGE) {
    Page &page = pager.getPage(current);

    auto &layout = page.layout->as<SlottedLayout>();

    for (SlotId slotIndex = 0; slotIndex < layout.slots.size(); ++slotIndex) {
      const Slot &slot = layout.slots[slotIndex];

      if (slot.state != SLOT_USED)
        continue;

      RawBytes entryBytes(page.data.begin() + slot.offset,
                          page.data.begin() + slot.offset + slot.size);

      IndexEntry entry = Serialization::deserializeIndexEntry(entryBytes);

      LoadedIndexEntry loaded{
          .entry = std::move(entry),
          .indexRef = {current, static_cast<SlotId>(slotIndex)}};

      allEntries.push_back(std::move(loaded));
    }

    pager.unpin(current);
    current = layout.header.nextPage;
  }

  index.rebuild(allEntries);
}
