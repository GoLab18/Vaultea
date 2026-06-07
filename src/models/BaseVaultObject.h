#pragma once

#include "storage/UUID.h"
#include "util/Helpers.h"

struct BaseVaultObject {
  UUID id;

  vault::util::time::EpochTime createdAt;
  vault::util::time::EpochTime updatedAt;
};
