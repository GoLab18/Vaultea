#pragma once

#include "UUID.h"
#include "models/IndexEntry.h"
#include "storage/UUID.h"

#include <absl/container/btree_map.h>
#include <vector>

// TODO we can create an interface for example "Index" that will have virtual
// methods regarding index operations and the BTreeIndex is gonna be the leading
// implementation

class BTreeIndex {
public:
  void insert(const IndexEntry &entry);

  void remove(const UUID &id);

  const IndexEntry *findEntry(const UUID &id);

  const IndexEntry *findFolder(const UUID &id);

  std::vector<IndexEntry> findByFolder(const UUID &folderId);

  std::vector<IndexEntry> findEntriesByName(const std::string &name);

  std::vector<IndexEntry> findFoldersByName(const std::string &name);

  std::vector<IndexEntry> all() const;

private:
  absl::btree_map<UUID, IndexEntry> byId;
  absl::btree_multimap<UUID, UUID> byFolder;

  absl::btree_multimap<std::string, UUID> byEntryName;
  absl::btree_multimap<std::string, UUID> byFolderName;

  const IndexEntry *find(const UUID &id);
};
