#include "DefaultCodec.h"
#include "Serialization.h"
#include "crypto/CryptoService.h"
#include "models/ProcessedBlob.h"
#include "pipeline/LZ4Compressor.h"
#include "pipeline/ZstdCompressor.h"

#include <stdexcept>

DefaultCodec::DefaultCodec(Key masterKey, CompressionType compType)
    : masterKey(masterKey) {
  switch (compType) {

  case CompressionType::LZ4:
    compressor = std::make_unique<LZ4Compressor>();
    break;

  case CompressionType::Zstd:
    compressor = std::make_unique<ZstdCompressor>();
    break;

  case CompressionType::None:
  default:
    throw std::invalid_argument(
        "A valid active compression type must be specified for the codec.");
  }
}

RawBytes DefaultCodec::encodeData(const RawBytes &plain) {
  auto compressionResult = compressor->compress(plain);

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

  return compressor->decompress(blob.compressionType, decrypted,
                                blob.originalSize);
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
