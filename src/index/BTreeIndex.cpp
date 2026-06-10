#include "BTreeIndex.h"
#include "util/Helpers.h"

void BTreeIndex::insert(const LoadedIndexEntry &loadedEntry) {
  byId[loadedEntry.entry.id] = loadedEntry;

  switch (loadedEntry.entry.type) {

  case IndexObjectType::Entry: {
    const auto &meta = std::get<ItemIndexMeta>(loadedEntry.entry.payload);

    byFolder.insert({meta.folderId, loadedEntry.entry.id});
    byEntryName.insert(
        {vault::util::normalize(meta.name), loadedEntry.entry.id});

    break;
  }

  case IndexObjectType::Folder: {
    const auto &meta = std::get<FolderIndexMeta>(loadedEntry.entry.payload);

    byFolderName.insert(
        {vault::util::normalize(meta.name), loadedEntry.entry.id});

    break;
  }
  }
}

void BTreeIndex::remove(const UUID &id) {
  auto it = byId.find(id);

  if (it == byId.end())
    return;

  const LoadedIndexEntry &loadedEntry = it->second;

  switch (loadedEntry.entry.type) {

  case IndexObjectType::Entry: {
    const auto &meta = std::get<ItemIndexMeta>(loadedEntry.entry.payload);

    auto range = byFolder.equal_range(meta.folderId);
    for (auto i = range.first; i != range.second; ++i) {
      if (i->second == id) {
        byFolder.erase(i);
        break;
      }
    }

    auto nameRange = byEntryName.equal_range(vault::util::normalize(meta.name));
    for (auto i = nameRange.first; i != nameRange.second; ++i) {
      if (i->second == id) {
        byEntryName.erase(i);
        break;
      }
    }

    break;
  }

  case IndexObjectType::Folder: {
    const auto &meta = std::get<FolderIndexMeta>(loadedEntry.entry.payload);

    auto range = byFolderName.equal_range(vault::util::normalize(meta.name));
    for (auto i = range.first; i != range.second; ++i) {
      if (i->second == id) {
        byFolderName.erase(i);
        break;
      }
    }

    break;
  }
  }

  byId.erase(it);
}

const LoadedIndexEntry *BTreeIndex::find(const UUID &id) const {
  auto it = byId.find(id);
  if (it == byId.end())
    return nullptr;

  return &it->second;
}

const IndexEntry *BTreeIndex::findEntry(const UUID &id) const {
  auto *e = find(id);

  if (!e)
    return nullptr;

  if (e->entry.type != IndexObjectType::Entry)
    return nullptr;

  return &e->entry;
}

const IndexEntry *BTreeIndex::findFolder(const UUID &id) const {
  auto *e = find(id);

  if (!e)
    return nullptr;

  if (e->entry.type != IndexObjectType::Folder)
    return nullptr;

  return &e->entry;
}

std::vector<IndexEntry> BTreeIndex::findByFolder(const UUID &folderId) const {
  std::vector<IndexEntry> out;

  auto range = byFolder.equal_range(folderId);

  for (auto it = range.first; it != range.second; ++it) {
    auto idIt = byId.find(it->second);
    if (idIt == byId.end())
      continue;

    out.push_back(idIt->second.entry);
  }

  return out;
}

std::vector<IndexEntry>
BTreeIndex::findEntriesByName(const std::string &name) const {
  std::vector<IndexEntry> out;

  const auto normalized = vault::util::normalize(name);

  auto end = vault::util::nextPrefix(normalized);

  auto it = byEntryName.lower_bound(normalized);
  auto itEnd = end.empty() ? byEntryName.end() : byEntryName.lower_bound(end);

  for (; it != itEnd; ++it) {
    out.push_back(byId.at(it->second).entry);
  }

  return out;
}

std::vector<IndexEntry>
BTreeIndex::findFoldersByName(const std::string &name) const {
  std::vector<IndexEntry> out;

  const auto normalized = vault::util::normalize(name);

  auto end = vault::util::nextPrefix(normalized);

  auto it = byFolderName.lower_bound(normalized);
  auto itEnd = end.empty() ? byFolderName.end() : byFolderName.lower_bound(end);

  for (; it != itEnd; ++it) {
    out.push_back(byId.at(it->second).entry);
  }

  return out;
}

std::vector<IndexEntry> BTreeIndex::all() const {
  std::vector<IndexEntry> out;
  out.reserve(byId.size());

  for (const auto &[id, loadedEntry] : byId) {
    out.push_back(loadedEntry.entry);
  }

  return out;
}

void BTreeIndex::rebuild(const std::vector<LoadedIndexEntry> &entries) {
  byId.clear();
  byFolder.clear();
  byEntryName.clear();
  byFolderName.clear();

  for (const auto &e : entries) {
    insert(e);
  }
}
