#pragma once

#include "storage/page/Constants.h"

#include <cstdint>

struct VaultHeader {
  vault::storage::page::PageId indexRootPage;
  vault::storage::page::PageId freelistRootPage;

  uint64_t entryCount;
  uint64_t folderCount;

  uint64_t createdAt;
  uint64_t updatedAt;
};
