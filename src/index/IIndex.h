#pragma once

#include <string>
#include <vector>

template <typename Key, typename Entry, typename LoadedEntry> class IIndex {
public:
  virtual ~IIndex() = default;

  virtual void insert(const LoadedEntry &loadedEntry) = 0;
  virtual void remove(const Key &id) = 0;

  virtual const LoadedEntry *find(const Key &id) const = 0;
  virtual const Entry *findEntry(const Key &id) const = 0;
  virtual const Entry *findFolder(const Key &id) const = 0;

  virtual std::vector<Entry> findByFolder(const Key &folderId) const = 0;
  virtual std::vector<Entry>
  findEntriesByName(const std::string &name) const = 0;
  virtual std::vector<Entry>
  findFoldersByName(const std::string &name) const = 0;

  virtual std::vector<Entry> all() const = 0;

  virtual void rebuild(const std::vector<LoadedEntry> &entries) = 0;
};
