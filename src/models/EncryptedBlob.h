#pragma once

#include "crypto/Constants.h"
#include "storage/Constants.h"

#include <array>
#include <cstdint>
#include <vector>

struct EncryptedBlob {
  vault::crypto::Nonce nonce;
  RawBytes ciphertext;
};
