#pragma once

#include "Constants.h"
#include "storage/Constants.h"

#include <cstdint>
#include <string>
#include <vector>

using namespace vault::storage;

class CryptoService {
public:
  vault::crypto::Key deriveMasterKey(const std::string &password,
                                     const vault::crypto::Salt &salt);

  RawBytes encrypt(const RawBytes &data, const vault::crypto::Key &key,
                   vault::crypto::Nonce &nonce);

  RawBytes decrypt(const RawBytes &cipher, const vault::crypto::Key &key,
                   const vault::crypto::Nonce &nonce);

  vault::crypto::Salt generateSalt();
};
