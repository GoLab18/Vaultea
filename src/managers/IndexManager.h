#pragma once

#include "SlottedManager.h"
#include "index/Config.h"
#include "models/IndexEntry.h"

#include <memory>
#include <vector>

class IndexManager : public SlottedManager {
public:
  explicit IndexManager(Pager &pager, PageId rootPage);

  void loadIndex(PageId rootIndexPage);

  void insert(const IndexEntry &entry);
  void update(const IndexEntry &entry);
  void remove(const UUID &id);

  const IndexEntry *findEntry(const UUID &id);
  const IndexEntry *findFolder(const UUID &id);

  std::vector<IndexEntry> findByFolder(const UUID &id);
  std::vector<IndexEntry> findEntriesByName(const std::string &query);
  std::vector<IndexEntry> findFoldersByName(const std::string &query);

  std::vector<IndexEntry> scanAll();

  PageId rootPage() const;

private:
  std::unique_ptr<VaultIndex> index;

  PageId allocateIndexPage();
  PageId findIndexPageWithSpace(uint16_t size);

  void persistEntry(const IndexEntry &entry);
  void erasePersistent(const UUID &id);
};
