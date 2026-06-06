#pragma once

#include "crypto/Constants.h"
#include "storage/Constants.h"

struct EncryptedBlob {
  vault::crypto::Nonce nonce;
  vault::storage::RawBytes ciphertext;
};
