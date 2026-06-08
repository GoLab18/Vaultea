#pragma once

#include "Constants.h"
#include "storage/Constants.h"

#include <string>

using namespace vault::crypto;
using namespace vault::storage;

class CryptoService {
public:
  static vault::crypto::Key deriveMasterKey(const std::string &password,
                                            const Salt &salt);

  static RawBytes encrypt(const RawBytes &data, const Key &key,
                          const Nonce &nonce);

  static RawBytes decrypt(const RawBytes &cipher, const Key &key,
                          const Nonce &nonce);

  static Nonce generateNonce();

  static Salt generateSalt();

  static KeyCheck deriveKeyCheck(const Key &key);
};
