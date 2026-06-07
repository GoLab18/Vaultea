#pragma once

#include "models/Folder.h"
#include "models/VaultEntry.h"
#include "storage/BTreeIndex.h"
#include "storage/UUID.h"

#include <unordered_map>
#include <vector>

class VaultRepository {
public:
  void addEntry(const VaultEntry &entry, const IndexEntry &indexEntry);

  void removeEntry(const UUID &id);

  VaultEntry *getEntry(const UUID &id);

  std::vector<VaultEntry> getAllEntries();

  std::vector<VaultEntry> getEntriesByFolder(const UUID &folderId);

  std::vector<VaultEntry> searchByName(const std::string &query);

  bool hasEntry(const UUID &id);

  void addFolder(const Folder &folder);

  void removeFolder(const UUID &id);

  Folder *getFolder(const UUID &id);

  std::vector<Folder> getFolders();

  std::vector<Folder> searchFolders(const std::string &query);

  bool hasFolder(const UUID &id);

  const BTreeIndex &indexView() const; // read-only for engine persistence

  void rebuildIndex(const std::vector<IndexEntry> &entries);

private:
  std::unordered_map<UUID, VaultEntry> entries;

  std::unordered_map<UUID, Folder> folders;

  BTreeIndex index;
};
