
#include "CryptoService.h"

#include <sodium.h>
#include <stdexcept>

using namespace vault::crypto;

Key CryptoService::deriveMasterKey(const std::string &password,
                                   const Salt &salt) {

  Key key{};

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
                                            const Key &key, Nonce &nonce) {

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
                                            const Key &key,
                                            const Nonce &nonce) {

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

Salt CryptoService::generateSalt() {
  Salt salt{};

  randombytes_buf(salt.data(), salt.size());

  return salt;
}
