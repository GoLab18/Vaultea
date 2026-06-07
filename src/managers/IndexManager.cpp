#include "IndexManager.h"
#include "pipeline/Serialization.h"

IndexManager::IndexManager(Pager &pager, PageId rootPage)
    : SlottedManager(pager, rootPage, PageType::Index) {
  index = std::make_unique<BTreeIndex>();

  loadIndex(rootPage);
}

void IndexManager::loadIndex(PageId rootIndexPage) {
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

  index->rebuild(allEntries);
}

void IndexManager::insert(const IndexEntry &entry) {
  auto bytes = Serialization::serializeIndexEntry(entry);

  RecordRef location = insertRecord(bytes);

  LoadedIndexEntry loaded{.entry = entry, .indexRef = location};

  index->insert(loaded);
}

void IndexManager::update(const IndexEntry &entry) {
  auto *record = index->find(entry.id);

  if (!record)
    return;

  auto bytes = Serialization::serializeIndexEntry(entry);

  auto moved = updateRecord(record->indexRef, bytes);

  LoadedIndexEntry updated{.entry = entry,
                           .indexRef = moved ? *moved : record->indexRef};

  index->remove(entry.id);
  index->insert(updated);
}

void IndexManager::remove(const UUID &id) {
  auto *record = index->find(id);

  if (!record)
    return;

  deleteRecord(record->indexRef);

  index->remove(id);
}

const IndexEntry *IndexManager::findEntry(const UUID &id) {
  return index->findEntry(id);
}

const IndexEntry *IndexManager::findFolder(const UUID &id) {
  return index->findFolder(id);
}

std::vector<IndexEntry> IndexManager::findByFolder(const UUID &id) {
  return index->findByFolder(id);
}

std::vector<IndexEntry>
IndexManager::findEntriesByName(const std::string &query) {
  return index->findEntriesByName(query);
}

std::vector<IndexEntry>
IndexManager::findFoldersByName(const std::string &query) {
  return index->findFoldersByName(query);
}

std::vector<IndexEntry> IndexManager::scanAll() { return index->all(); }
