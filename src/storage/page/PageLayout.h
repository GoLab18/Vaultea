#pragma once

#include "Constants.h"

#include <cstdint>
#include <stdexcept>
#include <vector>

using namespace vault::storage::page;

enum class PageType : uint8_t { Free, Data, Freelist, Index };

struct PageHeader {
  PageId pageId;
  PageType type;
  PageId nextPage;

  // TODO don't we need the previous page also (?) -> linked-list like behavior
  // for for example page freeing case

  // PageId prevPage;

  PageHeader()
      : pageId(INVALID_PAGE), type(PageType::Free), nextPage(INVALID_PAGE) {}
};

class PageLayoutBase {
public:
  PageHeader header;

  explicit PageLayoutBase(PageHeader h) : header(h) {}

  virtual ~PageLayoutBase() = default;

  template <typename T> T &as() {
    auto *ptr = dynamic_cast<T *>(this);
    if (!ptr)
      throw std::runtime_error("invalid layout cast");
    return *ptr;
  }

  template <typename T> const T &as() const {
    auto *ptr = dynamic_cast<const T *>(this);
    if (!ptr)
      throw std::runtime_error("invalid layout cast");
    return *ptr;
  }
};

enum class LayoutType : uint8_t { Slotted, Freelist, Free };

enum class SlotState : uint8_t { SlotUsed, SlotDeleted };

struct Slot {
  uint16_t offset;
  uint16_t size;
  SlotState state;
};

struct SlottedLayout : public PageLayoutBase {
  uint16_t lower = HEADER_SIZE;
  uint16_t upper = 0;

  std::vector<Slot> slots;

  explicit SlottedLayout(PageHeader h, uint32_t pageSize)
      : PageLayoutBase(h), upper(pageSize) {}
};

struct FreelistLayout : public PageLayoutBase {
  std::vector<PageId> freePages;

  explicit FreelistLayout(PageHeader h) : PageLayoutBase(h) {}
};

struct FreeLayout : public PageLayoutBase {
  explicit FreeLayout(PageHeader h) : PageLayoutBase(h) {}
};

constexpr LayoutType toLayoutType(PageType type) {
  switch (type) {
  case PageType::Data:
  case PageType::Index:
    return LayoutType::Slotted;

  case PageType::Freelist:
    return LayoutType::Freelist;

  case PageType::Free:
    return LayoutType::Free;
  }
}
