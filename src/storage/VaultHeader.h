#pragma once

#include "storage/page/Constants.h"
#include "util/Helpers.h"

#include <cstdint>

using namespace vault::storage::page;

struct VaultHeader {
  PageId indexRootPage;
  PageId dataRootPage;
  PageId freelistRootPage;

  uint64_t entryCount;
  uint64_t folderCount;

  vault::util::time::EpochTime createdAt;
  vault::util::time::EpochTime updatedAt;
};
