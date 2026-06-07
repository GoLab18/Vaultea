#include "VaultRepository.h"

void VaultRepository::addEntry(const VaultEntry &entry,
                               const IndexEntry &indexEntry) {
  entries[entry.id] = entry;

  index.insert(indexEntry);
}

void VaultRepository::removeEntry(const UUID &id) {
  entries.erase(id);

  index.remove(id);
}

VaultEntry *VaultRepository::getEntry(const UUID &id) {
  auto it = entries.find(id);

  if (it == entries.end()) {
    return nullptr;
  }

  return &it->second;
}

std::vector<VaultEntry> VaultRepository::getAllEntries() {
  std::vector<VaultEntry> out;

  out.reserve(entries.size());

  for (const auto &[id, entry] : entries) {
    out.push_back(entry);
  }

  return out;
}

std::vector<VaultEntry>
VaultRepository::getEntriesByFolder(const UUID &folderId) {
  std::vector<VaultEntry> out;

  auto indexEntries = index.findByFolder(folderId);

  out.reserve(indexEntries.size());

  for (const auto &idx : indexEntries) {
    auto it = entries.find(idx.id);

    if (it != entries.end()) {
      out.push_back(it->second);
    }
  }

  return out;
}

std::vector<VaultEntry>
VaultRepository::searchByName(const std::string &query) {
  std::vector<VaultEntry> out;

  for (const auto &[id, entry] : entries) {
    if (entry.name.find(query) != std::string::npos) {
      out.push_back(entry);
    }
  }

  return out;
}

bool VaultRepository::hasEntry(const UUID &id) {
  return entries.find(id) != entries.end();
}

void VaultRepository::addFolder(const Folder &folder) {
  folders[folder.id] = folder;
}

void VaultRepository::removeFolder(const UUID &id) { folders.erase(id); }

Folder *VaultRepository::getFolder(const UUID &id) {
  auto it = folders.find(id);

  if (it == folders.end()) {
    return nullptr;
  }

  return &it->second;
}

std::vector<Folder> VaultRepository::getFolders() {
  std::vector<Folder> out;

  out.reserve(folders.size());

  for (const auto &[id, folder] : folders) {
    out.push_back(folder);
  }

  return out;
}

std::vector<Folder> VaultRepository::searchFolders(const std::string &query) {
  std::vector<Folder> out;

  for (const auto &[id, folder] : folders) {
    if (folder.name.find(query) != std::string::npos) {
      out.push_back(folder);
    }
  }

  return out;
}

bool VaultRepository::hasFolder(const UUID &id) {
  return folders.find(id) != folders.end();
}

void VaultRepository::rebuildIndex(const std::vector<IndexEntry> &entries) {
  index.rebuild(entries);
}

const BTreeIndex &VaultRepository::indexView() const { return index; }
