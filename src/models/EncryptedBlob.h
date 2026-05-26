#pragma once

#include "crypto/Constants.h"

#include <array>
#include <cstdint>
#include <vector>

struct EncryptedBlob {
  vault::crypto::Nonce nonce;
  std::vector<uint8_t> ciphertext;
};
