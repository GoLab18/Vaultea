#pragma once

#include "models/ProcessedBlob.h"
#include "storage/Constants.h"

#include <cstdint>

using namespace vault::storage;

struct CompressionResult {
  CompressionType type;
  uint32_t originalSize;
  RawBytes data;
};

class ICompressor {
public:
  virtual ~ICompressor() = default;

  virtual CompressionResult compress(const RawBytes &input) = 0;

  virtual RawBytes decompress(CompressionType type, const RawBytes &input,
                              uint32_t originalSize) = 0;
};
