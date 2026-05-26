#pragma once

#include "storage/Constants.h"
#include "storage/UUID.h"

#include <string>

struct IndexEntry {
  IndexEntry();

  UUID id;
  std::string name;
  UUID folderId;

  vault::storage::PageId pageId;
  vault::storage::SlotId slotId;

  bool compressed;
};
