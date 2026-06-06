#pragma once

#include "Constants.h"

#include <cstdint>
#include <fstream>
#include <string>

class StorageEngine {
public:
  bool create(const std::string &path);
  bool open(const std::string &path);

  void close();
  void flush();

  bool isOpen() const;

  void write(uint64_t offset, const uint8_t *data, uint64_t size);

  vault::storage::RawBytes read(uint64_t offset, uint64_t size);

  uint64_t append(const uint8_t *data, uint64_t size);

  uint64_t fileSize();

private:
  std::fstream file;
};
