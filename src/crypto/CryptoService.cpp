#include "CryptoService.h"
#include "Constants.h"
#include "storage/Constants.h"

#include <array>
#include <sodium.h>
#include <stdexcept>

using namespace vault::crypto;

std::vector<uint8_t>
CryptoService::deriveMasterKey(const std::string &password,
                               const std::vector<uint8_t> &salt) {

  if (salt.size() != SALT_SIZE) {
    throw std::runtime_error("Invalid salt size");
  }

  std::vector<uint8_t> key(KEY_SIZE);

  int result = crypto_pwhash(
      key.data(), key.size(), password.c_str(), password.size(), salt.data(),
      crypto_pwhash_OPSLIMIT_MODERATE, crypto_pwhash_MEMLIMIT_MODERATE,
      crypto_pwhash_ALG_ARGON2ID13);

  if (result != 0) {
    throw std::runtime_error("Key derivation failed");
  }

  return key;
}

std::vector<uint8_t> CryptoService::encrypt(const std::vector<uint8_t> &data,
                                            const std::vector<uint8_t> &key,
                                            std::vector<uint8_t> &nonce) {

  if (key.size() != KEY_SIZE) {
    throw std::runtime_error("Invalid key size");
  }

  nonce.resize(NONCE_SIZE);

  randombytes_buf(nonce.data(), nonce.size());

  std::vector<uint8_t> cipher(data.size() + MAC_SIZE);

  unsigned long long cipherLen = 0;

  int result = crypto_aead_chacha20poly1305_ietf_encrypt(
      cipher.data(), &cipherLen, data.data(), data.size(), nullptr, 0, nullptr,
      nonce.data(), key.data());

  if (result != 0) {
    throw std::runtime_error("Encryption failed");
  }

  cipher.resize(cipherLen);

  return cipher;
}

std::vector<uint8_t> CryptoService::decrypt(const std::vector<uint8_t> &cipher,
                                            const std::vector<uint8_t> &key,
                                            const std::vector<uint8_t> &nonce) {

  if (key.size() != KEY_SIZE) {
    throw std::runtime_error("Invalid key size");
  }

  if (nonce.size() != NONCE_SIZE) {
    throw std::runtime_error("Invalid nonce size");
  }

  if (cipher.size() < MAC_SIZE) {
    throw std::runtime_error("Ciphertext too small");
  }

  std::vector<uint8_t> plain(cipher.size());

  unsigned long long plainLen = 0;

  int result = crypto_aead_chacha20poly1305_ietf_decrypt(
      plain.data(), &plainLen, nullptr, cipher.data(), cipher.size(), nullptr,
      0, nonce.data(), key.data());

  if (result != 0) {
    throw std::runtime_error(
        "Decryption failed: corrupted data or wrong password");
  }

  plain.resize(plainLen);

  return plain;
}

std::array<uint8_t, vault::storage::UUID_SIZE>
CryptoService::generateUUIDBytes() {
  std::array<uint8_t, vault::storage::UUID_SIZE> uuid;

  randombytes_buf(uuid.data(), uuid.size());

  // UUID v4 variant bits
  uuid[6] = (uuid[6] & 0x0F) | 0x40;
  uuid[8] = (uuid[8] & 0x3F) | 0x80;

  return uuid;
}

std::string CryptoService::generateUUID() {
  auto uuid = generateUUIDBytes();

  static constexpr char hex[] = "0123456789abcdef";

  std::string out;
  out.resize(36);

  int j = 0;

  for (int i = 0; i < 16; ++i) {
    if (i == 4 || i == 6 || i == 8 || i == 10) {
      out[j++] = '-';
    }

    out[j++] = hex[uuid[i] >> 4];
    out[j++] = hex[uuid[i] & 0x0F];
  }

  return out;
}
