#pragma once

#include "Constants.h"
#include "Pager.h"
#include "storage/BTreeIndex.h"

#include <vector>

using namespace vault::storage::page;

class PageLoader {
public:
  static std::vector<PageId> loadFreelist(PageId rootFreelistPage,
                                          Pager &pager);
  static void loadIndex(PageId rootIndexPage, BTreeIndex &index, Pager &pager);
};
