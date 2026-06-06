#pragma once

#include "Manager.h"
#include "models/RecordRef.h"

#include <optional>

using namespace vault::storage;

class SlottedManager : public Manager {
public:
  SlottedManager(Pager &pager, PageId rootPage, PageType pageType);

protected:
  RecordRef insertRecord(const RawBytes &bytes);

  RawBytes readRecord(const RecordRef &ref);

  std::optional<RecordRef> updateRecord(const RecordRef &ref,
                                        const RawBytes &bytes);

  void deleteRecord(const RecordRef &ref);

private:
  PageType pageType;

  PageId allocatePage();
  PageId findPageWithSpace(uint16_t size);
};
