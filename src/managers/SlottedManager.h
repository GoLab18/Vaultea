#pragma once

#include "Constants.h"
#include "models/RecordRef.h"
#include "storage/page/Constants.h"
#include "storage/page/Pager.h"

#include <map>
#include <optional>
#include <unordered_map>

using namespace vault::managers;
using namespace vault::storage;
using namespace vault::storage::page;

class SlottedManager {
public:
  explicit SlottedManager(Pager &pager, PageId rootPage, PageType pageType);

  PageId getRootPage() const;

protected:
  Pager &pager;

  RecordRef insertRecord(const RawBytes &bytes);

  RawBytes readRecord(const RecordRef &ref);

  std::optional<RecordRef> updateRecord(const RecordRef &ref,
                                        const RawBytes &bytes);

  void deleteRecord(const RecordRef &ref);

private:
  PageType pageType;

  PageId rootPage;
  PageId tailPage;

  std::multimap<FreeSpace, PageId> freeSpaceMap;
  std::unordered_map<PageId, std::multimap<FreeSpace, PageId>::iterator>
      pagePositions;

  void loadFreeSpaceMap();

  void freeEmptyPage(PageId id);
  void registerPage(PageId id, FreeSpace fs);
  void updatePageSpace(PageId id, FreeSpace newFs);
  void unregisterPage(PageId id);

  PageId allocatePage();
  PageId findPageWithSpace(uint16_t size);
};
