#pragma once

/*
===============================================================================
SERIALIZATION FORMAT (MVP NOTES)
===============================================================================

Current design:
- Binary serialization using std::memcpy for trivially copyable types
- Strings stored as: [uint32 length][raw bytes]
- Fields are written in fixed, manual order per object

===============================================================================
KNOWN LIMITATIONS (TO FIX LATER)
===============================================================================

1. Endianness
   - Currently assumes little-endian architecture
   - Not portable across different CPU architectures

   Fix: implement explicit LE encoding (write/read uint32/uint64 LE)

2. No versioning
   - Format is rigid; field order changes break compatibility

   Fix: add version field in VaultPreamble / per-struct versioning

3. No integrity checks
   - Corruption will not be detected

   Fix: add checksum (e.g., xxHash/SHA-256 per blob)

4. Unsafe reads
   - No bounds validation during deserialization

   Fix: validate buffer size before each read
*/

#include "models/Folder.h"
#include "models/VaultEntry.h"
#include "storage/BTreeIndex.h"

#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>
#include <vector>

class Serialization {
public:
  static std::vector<uint8_t> serializeEntry(const VaultEntry &entry);
  static VaultEntry deserializeEntry(const std::vector<uint8_t> &data);

  static std::vector<uint8_t> serializeFolder(const Folder &f);
  static Folder deserializeFolder(const std::vector<uint8_t> &data);

  static std::vector<uint8_t> serializeIndex(const BTreeIndex &index);
  static std::vector<IndexEntry>
  deserializeIndex(const std::vector<uint8_t> &data);

  static std::vector<uint8_t> serializePreamble(const VaultPreamble &p);
  static VaultPreamble deserializePreamble(const std::vector<uint8_t> &data);

  static std::vector<uint8_t> serializeHeader(const VaultHeader &h);
  static VaultHeader deserializeHeader(const std::vector<uint8_t> &data);

private:
  template <typename T>
  static void write(std::vector<uint8_t> &out, const T &value) {
    static_assert(std::is_trivially_copyable_v<T>,
                  "T must be trivially copyable");

    size_t old = out.size();
    out.resize(old + sizeof(T));

    std::memcpy(out.data() + old, &value, sizeof(T));
  }

  template <typename T>
  static T read(const std::vector<uint8_t> &data, size_t &offset) {
    static_assert(std::is_trivially_copyable_v<T>,
                  "T must be trivially copyable");

    if (offset + sizeof(T) > data.size()) {
      throw std::runtime_error("Serialization buffer underflow");
    }

    T value;

    std::memcpy(&value, data.data() + offset, sizeof(T));

    offset += sizeof(T);

    return value;
  }

  static void writeString(std::vector<uint8_t> &out, const std::string &str) {
    uint32_t size = static_cast<uint32_t>(str.size());
    write(out, size);

    size_t old = out.size();
    out.resize(old + size);

    std::memcpy(out.data() + old, str.data(), size);
  }

  static std::string readString(const std::vector<uint8_t> &data,
                                size_t &offset) {
    uint32_t size = read<uint32_t>(data, offset);

    if (offset + size > data.size()) {
      throw std::runtime_error("String exceeds serialization buffer");
    }

    std::string out(reinterpret_cast<const char *>(data.data() + offset), size);

    offset += size;

    return out;
  }

  template <typename T>
  static void readInto(const std::vector<uint8_t> &data, size_t &offset,
                       T &out) {
    static_assert(std::is_trivially_copyable_v<T>,
                  "T must be trivially copyable");

    if (offset + sizeof(T) > data.size()) {
      throw std::runtime_error("Serialization buffer underflow");
    }

    std::memcpy(&out, data.data() + offset, sizeof(T));

    offset += sizeof(T);
  }
};
