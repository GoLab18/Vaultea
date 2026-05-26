#include "BTreeIndex.h"

void BTreeIndex::insert(const IndexEntry &entry) {
  byId[entry.id] = entry;
  byFolder.insert({entry.folderId, entry.id});
}

void BTreeIndex::remove(const UUID &id) {
  auto it = byId.find(id);
  if (it == byId.end())
    return;

  const UUID &folderId = it->second.folderId;

  auto range = byFolder.equal_range(folderId);

  for (auto i = range.first; i != range.second; ++i) {
    if (i->second == id) {
      byFolder.erase(i);
      break;
    }
  }

  byId.erase(it);
}

IndexEntry *BTreeIndex::find(const UUID &id) {
  auto it = byId.find(id);
  if (it == byId.end())
    return nullptr;
  return &it->second;
}

std::vector<IndexEntry> BTreeIndex::findByFolder(const UUID &folderId) {
  std::vector<IndexEntry> out;

  auto range = byFolder.equal_range(folderId);

  for (auto it = range.first; it != range.second; ++it) {
    out.push_back(byId[it->second]);
  }

  return out;
}

std::vector<IndexEntry> BTreeIndex::all() const {
  std::vector<IndexEntry> out;
  out.reserve(byId.size());

  for (const auto &[id, entry] : byId) {
    out.push_back(entry);
  }

  return out;
}

void BTreeIndex::rebuild(const std::vector<IndexEntry> &entries) {
  byId.clear();
  byFolder.clear();

  for (const auto &e : entries) {
    byId[e.id] = e;
    byFolder.insert({e.folderId, e.id});
  }
}
