#include "BTreeIndex.h"

void BTreeIndex::insert(const LoadedIndexEntry &loadedEntry) {
  byId[loadedEntry.entry.id] = loadedEntry;

  switch (loadedEntry.entry.type) {

  case IndexObjectType::Entry: {
    const auto &meta = std::get<ItemIndexMeta>(loadedEntry.entry.payload);

    byFolder.insert({meta.folderId, loadedEntry.entry.id});
    byEntryName.insert({meta.name, loadedEntry.entry.id});

    break;
  }

  case IndexObjectType::Folder: {
    const auto &meta = std::get<FolderIndexMeta>(loadedEntry.entry.payload);

    byFolderName.insert({meta.name, loadedEntry.entry.id});

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

    auto nameRange = byEntryName.equal_range(meta.name);

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

    auto range = byFolderName.equal_range(meta.name);

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

const IndexEntry *BTreeIndex::find(const UUID &id) {
  auto it = byId.find(id);
  if (it == byId.end())
    return nullptr;
  return &it->second.entry;
}

const IndexEntry *BTreeIndex::findEntry(const UUID &id) {
  auto *e = find(id);

  if (!e)
    return nullptr;

  if (e->type != IndexObjectType::Entry)
    return nullptr;

  return e;
}

const IndexEntry *BTreeIndex::findFolder(const UUID &id) {
  auto *e = find(id);

  if (!e)
    return nullptr;

  if (e->type != IndexObjectType::Folder)
    return nullptr;

  return e;
}

std::vector<IndexEntry> BTreeIndex::findByFolder(const UUID &folderId) {
  std::vector<IndexEntry> out;

  auto range = byFolder.equal_range(folderId);

  for (auto it = range.first; it != range.second; ++it) {
    out.push_back(byId[it->second].entry);
  }

  return out;
}

std::vector<IndexEntry> BTreeIndex::findEntriesByName(const std::string &name) {
  std::vector<IndexEntry> out;

  auto range = byEntryName.equal_range(name);

  for (auto it = range.first; it != range.second; ++it) {
    out.push_back(byId[it->second].entry);
  }

  return out;
}

std::vector<IndexEntry> BTreeIndex::findFoldersByName(const std::string &name) {
  std::vector<IndexEntry> out;

  auto range = byFolderName.equal_range(name);

  for (auto it = range.first; it != range.second; ++it) {
    out.push_back(byId[it->second].entry);
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
