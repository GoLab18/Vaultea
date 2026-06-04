#include "PageFactory.h"
#include "FreePagePolicy.h"
#include "FreelistPagePolicy.h"
#include "PageLayout.h"
#include "PagePolicy.h"
#include "SlottedPagePolicy.h"

#include <memory>
#include <stdexcept>

std::unique_ptr<PageLayoutBase>
PageFactory::createLayout(PageId id, PageType type, uint32_t pageSize) {

  switch (type) {

  case PageType::Data:
  case PageType::Index:
    return std::make_unique<SlottedLayout>(id, type, pageSize);

  case PageType::Freelist:
    return std::make_unique<FreelistLayout>(id);

  case PageType::Free:
    return std::make_unique<FreeLayout>(id);
  }

  throw std::runtime_error("unknown page type");
}

std::unique_ptr<PagePolicy> PageFactory::createPolicy(PageType type) {

  switch (type) {

  case PageType::Data:
  case PageType::Index:
    return std::make_unique<SlottedPagePolicy>();

  case PageType::Freelist:
    return std::make_unique<FreelistPagePolicy>();

  case PageType::Free:
    return std::make_unique<FreePagePolicy>();
  }

  throw std::runtime_error("unknown page type");
}
