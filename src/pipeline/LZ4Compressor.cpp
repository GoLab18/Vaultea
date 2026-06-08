#include "LZ4Compressor.h"

#include <lz4.h>
#include <stdexcept>

CompressionResult LZ4Compressor::compress(const RawBytes &input) {
  if (input.empty()) {
    return {.type = CompressionType::None, .originalSize = 0, .data = {}};
  }

  int maxSize = LZ4_compressBound(static_cast<int>(input.size()));
  RawBytes output(maxSize);

  int compressedSize =
      LZ4_compress_default(reinterpret_cast<const char *>(input.data()),
                           reinterpret_cast<char *>(output.data()),
                           static_cast<int>(input.size()), maxSize);

  if (compressedSize <= 0) {
    throw std::runtime_error("LZ4 compression failed");
  }

  output.resize(compressedSize);

  auto storeRaw = compressedSize >= static_cast<int>(input.size());
  if (storeRaw) {
    return {.type = CompressionType::None,
            .originalSize = static_cast<uint32_t>(input.size()),
            .data = input};
  }

  return {.type = CompressionType::LZ4,
          .originalSize = static_cast<uint32_t>(input.size()),
          .data = std::move(output)};
}

RawBytes LZ4Compressor::decompress(CompressionType type, const RawBytes &input,
                                   uint32_t originalSize) {
  if (type == CompressionType::None) {
    return input;
  }

  if (type != CompressionType::LZ4) {
    throw std::runtime_error("Compression type must be LZ4 or none");
  }

  RawBytes output(originalSize);

  int result = LZ4_decompress_safe(reinterpret_cast<const char *>(input.data()),
                                   reinterpret_cast<char *>(output.data()),
                                   static_cast<int>(input.size()),
                                   static_cast<int>(originalSize));

  if (result < 0) {
    throw std::runtime_error("LZ4 decompression failed");
  }

  return output;
}
