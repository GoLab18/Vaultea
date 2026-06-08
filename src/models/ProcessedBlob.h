#pragma once

#include "crypto/Constants.h"
#include "storage/Constants.h"

#include <cstdint>

enum class CompressionType : uint8_t { None, LZ4 };

struct ProcessedBlob {
  CompressionType compressionType;
  uint32_t originalSize;

  vault::crypto::Nonce nonce;
  vault::storage::RawBytes ciphertext;
};
