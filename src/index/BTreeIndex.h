#pragma once

#include "IIndex.h"
#include "models/IndexEntry.h"
#include "storage/UUID.h"

#include <absl/container/btree_map.h>
#include <vector>

using BTreeIndexTemplate = IIndex<UUID, IndexEntry, LoadedIndexEntry>;

class BTreeIndex : public BTreeIndexTemplate {
public:
  void insert(const LoadedIndexEntry &loadedEntry) override;
  void remove(const UUID &id) override;

  const LoadedIndexEntry *find(const UUID &id) const override;
  const IndexEntry *findEntry(const UUID &id) const override;
  const IndexEntry *findFolder(const UUID &id) const override;

  std::vector<IndexEntry> findByFolder(const UUID &folderId) const override;
  std::vector<IndexEntry>
  findEntriesByName(const std::string &name) const override;
  std::vector<IndexEntry>
  findFoldersByName(const std::string &name) const override;

  std::vector<IndexEntry> all() const override;

  void rebuild(const std::vector<LoadedIndexEntry> &entries) override;

private:
  absl::btree_map<UUID, LoadedIndexEntry> byId;
  absl::btree_multimap<UUID, UUID> byFolder;

  absl::btree_multimap<std::string, UUID> byEntryName;
  absl::btree_multimap<std::string, UUID> byFolderName;
};
