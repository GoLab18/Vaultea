#pragma once

#include "models/IndexEntry.h"
#include "storage/UUID.h"

#include <absl/container/btree_map.h>

class BTreeIndex {
public:
  void insert(const IndexEntry &entry);

  void remove(const UUID &id);

  IndexEntry *find(const UUID &id);

  std::vector<IndexEntry> findByFolder(const UUID &folderId);

  std::vector<IndexEntry> all() const;

  void rebuild(const std::vector<IndexEntry> &entries);

private:
  absl::btree_map<UUID, IndexEntry> byId;
  absl::btree_multimap<UUID, UUID> byFolder;
};
