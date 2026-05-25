#pragma once

#include "Constants.h"

#include <cstdint>
#include <string>
#include <vector>

class CryptoService {
public:
  vault::crypto::Key deriveMasterKey(const std::string &password,
                                     const vault::crypto::Salt &salt);

  std::vector<uint8_t> encrypt(const std::vector<uint8_t> &data,
                               const vault::crypto::Key &key,
                               vault::crypto::Nonce &nonce);

  std::vector<uint8_t> decrypt(const std::vector<uint8_t> &cipher,
                               const vault::crypto::Key &key,
                               const vault::crypto::Nonce &nonce);

  vault::crypto::Salt generateSalt();
};
