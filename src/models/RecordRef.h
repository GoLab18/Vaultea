#pragma once

#include "storage/page/Constants.h"

using namespace vault::storage::page;

struct RecordRef {
  PageId pageId;
  SlotId slotId;
};
