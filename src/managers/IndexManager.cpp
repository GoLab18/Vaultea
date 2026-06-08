#include "IndexManager.h"
#include "pipeline/Serialization.h"

IndexManager::IndexManager(Pager &pager, PageId rootPage,
                           std::vector<LoadedIndexEntry> loadedEntries)
    : SlottedManager(pager, rootPage, PageType::Index) {
  index = std::make_unique<BTreeIndex>();
  index->rebuild(loadedEntries);
}

void IndexManager::insert(const IndexEntry &entry,
                          const RawBytes &processedBytes) {
  RecordRef location = insertRecord(processedBytes);

  LoadedIndexEntry loaded{.entry = entry, .indexRef = location};

  index->insert(loaded);
}

void IndexManager::update(const IndexEntry &entry,
                          const RawBytes &processedBytes) {
  auto *record = index->find(entry.id);

  if (!record)
    return;

  auto moved = updateRecord(record->indexRef, processedBytes);

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

PageId IndexManager::rootPage() const { return getRootPage(); }
