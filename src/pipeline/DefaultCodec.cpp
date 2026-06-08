#include "DefaultCodec.h"
#include "Serialization.h"
#include "crypto/CryptoService.h"
#include "models/EncryptedBlob.h"
#include "pipeline/Compression.h"

DefaultCodec::DefaultCodec(Key masterKey) : masterKey(masterKey) {}

RawBytes DefaultCodec::encodeData(const RawBytes &plain) {
  auto compressed = Compression::compress(plain);

  auto nonce = CryptoService::generateNonce();
  auto ciphertext = CryptoService::encrypt(compressed, masterKey, nonce);

  EncryptedBlob blob{.nonce = nonce, .ciphertext = ciphertext};

  return Serialization::serializeEncryptedBlob(blob);
}

RawBytes DefaultCodec::decodeData(const RawBytes &encoded) {
  auto blob = Serialization::deserializeEncryptedBlob(encoded);

  auto decrypted =
      CryptoService::decrypt(blob.ciphertext, masterKey, blob.nonce);

  return Compression::decompress(decrypted);
}

RawBytes DefaultCodec::encodeIndex(const RawBytes &plain) {
  auto nonce = CryptoService::generateNonce();
  auto ciphertext = CryptoService::encrypt(plain, masterKey, nonce);

  EncryptedBlob blob{.nonce = nonce, .ciphertext = ciphertext};

  return Serialization::serializeEncryptedBlob(blob);
}

RawBytes DefaultCodec::decodeIndex(const RawBytes &encoded) {
  auto blob = Serialization::deserializeEncryptedBlob(encoded);

  return CryptoService::decrypt(blob.ciphertext, masterKey, blob.nonce);
}
