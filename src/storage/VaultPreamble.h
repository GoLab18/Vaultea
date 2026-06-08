#pragma once

#include "Constants.h"
#include "UUID.h"
#include "crypto/Constants.h"

#include <array>
#include <cstdint>

using namespace vault::storage;

struct VaultPreamble {
  std::array<uint8_t, MAGIC_SIZE> magic;

  uint32_t version;

  UUID uuid;

  vault::crypto::Salt salt;
  vault::crypto::KeyCheck keyCheck;

  uint32_t pageSize;
};
