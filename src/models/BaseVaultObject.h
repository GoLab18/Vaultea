#pragma once

#include "storage/UUID.h"

#include <cstdint>

struct BaseVaultObject {
  UUID id;

  uint64_t createdAt;
  uint64_t updatedAt;
};
