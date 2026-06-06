#pragma once

#include "storage/page/Constants.h"

#include <cstdint>

using namespace vault::storage::page;

struct VaultHeader {
  PageId indexRootPage;
  PageId freelistRootPage;
  PageId dataRootPage;

  uint64_t entryCount;
  uint64_t folderCount;

  uint64_t createdAt;
  uint64_t updatedAt;
};
