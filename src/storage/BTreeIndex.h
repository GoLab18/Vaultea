#pragma once

#include "UUID.h"
#include "models/IndexEntry.h"
#include "storage/UUID.h"

#include <absl/container/btree_map.h>
#include <vector>

class BTreeIndex {
public:
    void insert(const LoadedIndexEntry &loadedEntry);

  void remove(const UUID &id);

  const IndexEntry *findEntry(const UUID &id);

  const IndexEntry *findFolder(const UUID &id);

  const IndexEntry *find(const UUID &id);

  std::vector<IndexEntry> findByFolder(const UUID &folderId);

  std::vector<IndexEntry> findEntriesByName(const std::string &name);

  std::vector<IndexEntry> findFoldersByName(const std::string &name);

  std::vector<IndexEntry> all() const;

  void rebuild(const std::vector<LoadedIndexEntry> &entries);

private:
  absl::btree_map<UUID, LoadedIndexEntry> byId;
  absl::btree_multimap<UUID, UUID> byFolder;

  absl::btree_multimap<std::string, UUID> byEntryName;
  absl::btree_multimap<std::string, UUID> byFolderName;
};
