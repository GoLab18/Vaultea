#include "VaultLoader.h"
#include "pipeline/Serialization.h"
#include "storage/page/PageLayout.h"

std::vector<LoadedIndexEntry>
VaultLoader::loadIndex(Pager &pager, PageId rootPage, Codec &codec) {
  std::vector<LoadedIndexEntry> result;

  PageId current = rootPage;

  while (current != INVALID_PAGE) {
    Page &page = pager.getPage(current);

    auto &layout = page.layout->as<SlottedLayout>();

    for (SlotId slotIndex = 0; slotIndex < layout.slots.size(); ++slotIndex) {
      const Slot &slot = layout.slots[slotIndex];

      if (slot.state != SlotState::SlotUsed)
        continue;

      RawBytes encoded(page.data.begin() + slot.offset,
                       page.data.begin() + slot.offset + slot.size);

      RawBytes plain = codec.decodeIndex(encoded);

      IndexEntry entry = Serialization::deserializeIndexEntry(plain);

      result.push_back(
          {.entry = std::move(entry), .indexRef = {current, slotIndex}});
    }

    pager.unpin(current);
    current = layout.header.nextPage;
  }

  return result;
}
