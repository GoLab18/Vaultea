#pragma once

#include "pipeline/ICompressor.h"

class LZ4Compressor : public ICompressor {
public:
  CompressionResult compress(const RawBytes &input) override;

  RawBytes decompress(CompressionType type, const RawBytes &input,
                      uint32_t originalSize) override;
};
