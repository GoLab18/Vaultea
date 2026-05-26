#pragma once

#include <cstdint>

struct VaultHeader {
  uint64_t indexPage;
  uint64_t dataPage;
  uint64_t freeListPage;

  uint64_t entryCount;
  uint64_t folderCount;

  uint64_t createdAt;
  uint64_t updatedAt;
};
