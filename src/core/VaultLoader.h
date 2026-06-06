#pragma once

#include "managers/Constants.h"
#include "storage/BTreeIndex.h"
#include "storage/page/Constants.h"
#include "storage/page/Pager.h"

#include <vector>

using namespace vault::managers;
using namespace vault::storage::page;

class VaultLoader {
public:
  static std::vector<PageId> loadFreelist(PageId rootFreelistPage,
                                          Pager &pager);
  static void loadIndex(PageId rootIndexPage, BTreeIndex &index, Pager &pager);
};
