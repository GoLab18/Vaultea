#include "CryptoService.h"

#include <cstring>
#include <sodium.h>
#include <stdexcept>

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

RawBytes CryptoService::encrypt(const RawBytes &data, const Key &key,
                                const Nonce &nonce) {
  RawBytes cipher(data.size() + MAC_SIZE);

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

RawBytes CryptoService::decrypt(const RawBytes &cipher, const Key &key,
                                const Nonce &nonce) {
  RawBytes plain(cipher.size());

  unsigned long long plainLen = 0;

  int result = crypto_aead_chacha20poly1305_ietf_decrypt(
      plain.data(), &plainLen, nullptr, cipher.data(), cipher.size(), nullptr,
      0, nonce.data(), key.data());

  if (result != 0) {
    throw std::runtime_error("Decryption failed");
  }

  plain.resize(plainLen);
  return plain;
}

Nonce CryptoService::generateNonce() {
  Nonce nonce;
  randombytes_buf(nonce.data(), nonce.size());

  return nonce;
}

Salt CryptoService::generateSalt() {
  Salt salt;
  randombytes_buf(salt.data(), salt.size());

  return salt;
}

KeyCheck CryptoService::deriveKeyCheck(const Key &key) {
  KeyCheck out;

  crypto_auth_hmacsha256_state state;
  crypto_auth_hmacsha256_init(&state, key.data(), key.size());

  crypto_auth_hmacsha256_update(
      &state, reinterpret_cast<const unsigned char *>(KEY_CHECK_CONTEXT),
      strlen(KEY_CHECK_CONTEXT));

  crypto_auth_hmacsha256_final(&state, out.data());

  return out;
}
