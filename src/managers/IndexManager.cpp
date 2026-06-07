#include "IndexManager.h"
#include "pipeline/Serialization.h"

IndexManager::IndexManager(Pager &pager, PageId rootPage)
    : SlottedManager(pager, rootPage, PageType::Index) {}

void IndexManager::insert(const IndexEntry &entry) {
  auto bytes = Serialization::serializeIndexEntry(entry);

  RecordRef location = insertRecord(bytes);

  LoadedIndexEntry loaded{.entry = entry, .indexRef = location};

  index.insert(loaded);
}

void IndexManager::update(const IndexEntry &entry) {
  auto *record = index.find(entry.id);

  if (!record)
    return;

  auto bytes = Serialization::serializeIndexEntry(entry);

  auto moved = updateRecord(record->indexRef, bytes);

  LoadedIndexEntry updated{.entry = entry,
                           .indexRef = moved ? *moved : record->indexRef};

  index.remove(entry.id);
  index.insert(updated);
}

void IndexManager::remove(const UUID &id) {
  auto *record = index.find(id);

  if (!record)
    return;

  deleteRecord(record->indexRef);

  index.remove(id);
}

const IndexEntry *IndexManager::find(const UUID &id) {
  return index.findEntry(id);
}

std::vector<IndexEntry> IndexManager::findByFolder(const UUID &id) {
  return index.findByFolder(id);
}

std::vector<IndexEntry> IndexManager::findByName(const std::string &q) {
  return index.findEntriesByName(q);
}

std::vector<IndexEntry> IndexManager::scanAll() { return index.all(); }
