#pragma once

#include "SlottedManager.h"
#include "models/IndexEntry.h"
#include "storage/BTreeIndex.h"

#include <vector>

class IndexManager : public SlottedManager {
public:
  explicit IndexManager(Pager &pager, PageId rootPage);

  void insert(const IndexEntry &entry);
  void update(const IndexEntry &entry);
  void remove(const UUID &id);

  const IndexEntry *find(const UUID &id);

  std::vector<IndexEntry> findByFolder(const UUID &id);
  std::vector<IndexEntry> findByName(const std::string &query);

  std::vector<IndexEntry> scanAll();

private:
  BTreeIndex index;

  PageId allocateIndexPage();
  PageId findIndexPageWithSpace(uint16_t size);

  void persistEntry(const IndexEntry &entry);
  void erasePersistent(const UUID &id);
};
