#pragma once

#include "models/IndexEntry.h"
#include "pipeline/Codec.h"
#include "storage/page/Pager.h"

#include <vector>

class VaultLoader {
public:
  static std::vector<LoadedIndexEntry> loadIndex(Pager &pager, PageId rootPage,
                                                 Codec &codec);
};
