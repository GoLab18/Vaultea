#include "ZstdCompressor.h"

#include <stdexcept>
#include <zstd.h>

CompressionResult ZstdCompressor::compress(const RawBytes &input) {
  if (input.empty()) {
    return {.type = CompressionType::None, .originalSize = 0, .data = {}};
  }

  size_t maxSize = ZSTD_compressBound(input.size());
  RawBytes output(maxSize);

  size_t compressedSize =
      ZSTD_compress(output.data(), maxSize, input.data(), input.size(), 1);

  if (ZSTD_isError(compressedSize)) {
    throw std::runtime_error(std::string("Zstd compression failed: ") +
                             ZSTD_getErrorName(compressedSize));
  }

  output.resize(compressedSize);

  bool storeRaw = compressedSize >= input.size();
  if (storeRaw) {
    return {.type = CompressionType::None,
            .originalSize = static_cast<uint32_t>(input.size()),
            .data = input};
  }

  return {.type = CompressionType::Zstd,
          .originalSize = static_cast<uint32_t>(input.size()),
          .data = std::move(output)};
}

RawBytes ZstdCompressor::decompress(CompressionType type, const RawBytes &input,
                                    uint32_t originalSize) {
  if (type == CompressionType::None) {
    return input;
  }

  if (type != CompressionType::Zstd) {
    throw std::runtime_error("Compression type must be Zstd or none");
  }

  RawBytes output(originalSize);

  size_t result =
      ZSTD_decompress(output.data(), originalSize, input.data(), input.size());

  if (ZSTD_isError(result)) {
    throw std::runtime_error(std::string("Zstd decompression failed: ") +
                             ZSTD_getErrorName(result));
  }

  return output;
}
