#include "DefaultCodec.h"
#include "Serialization.h"
#include "crypto/CryptoService.h"
#include "models/ProcessedBlob.h"
#include "pipeline/LZ4Compressor.h"
#include "pipeline/ZstdCompressor.h"

#include <stdexcept>

DefaultCodec::DefaultCodec(Key masterKey)
    : masterKey(masterKey), lz4(std::make_unique<LZ4Compressor>()),
      zstd(std::make_unique<ZstdCompressor>()) {}

RawBytes DefaultCodec::encodeData(const RawBytes &plain) {
  CompressionResult compressionResult;

  if (plain.size() < 512) {
    compressionResult = {.type = CompressionType::None,
                         .originalSize = static_cast<uint32_t>(plain.size()),
                         .data = plain};

  } else if (plain.size() >= 512 && plain.size() < 65536) { // 512B to 64KB
    compressionResult = lz4->compress(plain);
  } else {
    compressionResult = zstd->compress(plain);
  }

  auto nonce = CryptoService::generateNonce();
  auto ciphertext =
      CryptoService::encrypt(compressionResult.data, masterKey, nonce);

  ProcessedBlob blob{.compressionType = compressionResult.type,
                     .originalSize = compressionResult.originalSize,
                     .nonce = nonce,
                     .ciphertext = ciphertext};

  return Serialization::serializeProcessedBlob(blob);
}

RawBytes DefaultCodec::decodeData(const RawBytes &encoded) {
  auto blob = Serialization::deserializeProcessedBlob(encoded);
  auto decrypted =
      CryptoService::decrypt(blob.ciphertext, masterKey, blob.nonce);

  switch (blob.compressionType) {
  case CompressionType::None:
    return decrypted;
  case CompressionType::LZ4:
    return lz4->decompress(blob.compressionType, decrypted, blob.originalSize);
  case CompressionType::Zstd:
    return zstd->decompress(blob.compressionType, decrypted, blob.originalSize);
  default:
    throw std::runtime_error("Unknown compression type detected in blob");
  }
}

RawBytes DefaultCodec::encodeIndex(const RawBytes &plain) {
  auto nonce = CryptoService::generateNonce();
  auto ciphertext = CryptoService::encrypt(plain, masterKey, nonce);

  ProcessedBlob blob{.compressionType = CompressionType::None,
                     .originalSize = static_cast<uint32_t>(plain.size()),
                     .nonce = nonce,
                     .ciphertext = ciphertext};

  return Serialization::serializeProcessedBlob(blob);
}

RawBytes DefaultCodec::decodeIndex(const RawBytes &encoded) {
  auto blob = Serialization::deserializeProcessedBlob(encoded);

  return CryptoService::decrypt(blob.ciphertext, masterKey, blob.nonce);
}

RawBytes DefaultCodec::encodeHeader(const RawBytes &plain) {
  auto nonce = CryptoService::generateNonce();
  auto ciphertext = CryptoService::encrypt(plain, masterKey, nonce);

  ProcessedBlob blob{.compressionType = CompressionType::None,
                     .originalSize = static_cast<uint32_t>(plain.size()),
                     .nonce = nonce,
                     .ciphertext = ciphertext};

  return Serialization::serializeProcessedBlob(blob);
}

RawBytes DefaultCodec::decodeHeader(const RawBytes &encoded) {
  auto blob = Serialization::deserializeProcessedBlob(encoded);

  return CryptoService::decrypt(blob.ciphertext, masterKey, blob.nonce);
}
