#pragma once

#include "SlottedManager.h"
#include "models/RecordRef.h"

#include <optional>

class DataManager : public SlottedManager {
public:
  DataManager(Pager &pager, PageId rootPage);

  RecordRef insert(const RawBytes &data);

  RawBytes read(const RecordRef &ref);

  std::optional<RecordRef> update(const RecordRef &ref, const RawBytes &data);

  void remove(const RecordRef &ref);
};
