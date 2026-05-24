#include "StorageEngine.h"

#include <stdexcept>

bool StorageEngine::create(const std::string &path) {
  file.open(path, std::ios::binary | std::ios::out | std::ios::trunc);

  file.close();

  file.open(path, std::ios::binary | std::ios::in | std::ios::out);

  return file.is_open();
}

bool StorageEngine::open(const std::string &path) {
  file.open(path, std::ios::binary | std::ios::in | std::ios::out);

  return file.is_open();
}

void StorageEngine::close() {
  if (file.is_open()) {
    file.close();
  }
}

void StorageEngine::flush() { file.flush(); }

bool StorageEngine::isOpen() const { return file.is_open(); }

void StorageEngine::write(uint64_t offset, const uint8_t *data, uint64_t size) {
  file.seekp(offset);

  file.write(reinterpret_cast<const char *>(data), size);

  if (!file.good()) {
    throw std::runtime_error("StorageEngine write failed");
  }
}

std::vector<uint8_t> StorageEngine::read(uint64_t offset, uint64_t size) {
  std::vector<uint8_t> out(size);

  file.seekg(offset);

  file.read(reinterpret_cast<char *>(out.data()), size);

  if (!file.good()) {
    throw std::runtime_error("StorageEngine read failed");
  }

  return out;
}

uint64_t StorageEngine::append(const uint8_t *data, uint64_t size) {
  file.seekp(0, std::ios::end);

  uint64_t offset = static_cast<uint64_t>(file.tellp());

  file.write(reinterpret_cast<const char *>(data), size);

  if (!file.good()) {
    throw std::runtime_error("StorageEngine append failed");
  }

  return offset;
}

uint64_t StorageEngine::fileSize() {
  file.seekg(0, std::ios::end);

  return static_cast<uint64_t>(file.tellg());
}
