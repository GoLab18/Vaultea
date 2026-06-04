#pragma once

#include "Constants.h"

#include <cstdint>

struct VaultLayout {
  uint64_t preambleOffset = 0;

  uint64_t headerOffset = vault::storage::VAULT_PREAMBLE_SIZE;

  uint64_t pageRegionOffset =
      vault::storage::VAULT_PREAMBLE_SIZE + vault::storage::VAULT_HEADER_SIZE;
};
