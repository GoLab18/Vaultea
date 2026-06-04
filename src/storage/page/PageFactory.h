#pragma once

#include "PageLayout.h"
#include "PagePolicy.h"

#include <memory>

class PageFactory {
public:
  static std::unique_ptr<PageLayoutBase> createLayout(PageId id, PageType type,
                                                      uint32_t pageSize);

  static std::unique_ptr<PagePolicy> createPolicy(PageType type);
};
