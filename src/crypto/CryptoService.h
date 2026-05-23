#pragma once

#include "storage/Constants.h"

#include <cstdint>
#include <string>
#include <vector>

class CryptoService {
public:
  std::vector<uint8_t> deriveMasterKey(const std::string &password,
                                       const std::vector<uint8_t> &salt);

  std::vector<uint8_t> encrypt(const std::vector<uint8_t> &data,
                               const std::vector<uint8_t> &key,
                               std::vector<uint8_t> &nonce);

  std::vector<uint8_t> decrypt(const std::vector<uint8_t> &cipher,
                               const std::vector<uint8_t> &key,
                               const std::vector<uint8_t> &nonce);

  std::array<uint8_t, vault::storage::UUID_SIZE> generateUUIDBytes();

  std::string generateUUID();
};
