#pragma once

/*
===============================================================================
SERIALIZATION FORMAT (MVP)
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
*/

#include "models/EncryptedBlob.h"
#include "models/Folder.h"
#include "models/IndexEntry.h"
#include "models/VaultEntry.h"
#include "storage/Constants.h"
#include "storage/VaultHeader.h"
#include "storage/VaultPreamble.h"
#include "storage/page/PageLayout.h"

#include <cstring>
#include <string>
#include <type_traits>

using namespace vault::storage;
using namespace vault::storage::page;

class Serialization {
public:
  static RawBytes serializeEntry(const VaultEntry &entry);
  static VaultEntry deserializeEntry(const RawBytes &data);

  static RawBytes serializeFolder(const Folder &f);
  static Folder deserializeFolder(const RawBytes &data);

  static RawBytes serializeIndexEntry(const IndexEntry &e);
  static IndexEntry deserializeIndexEntry(const RawBytes &data);

  static RawBytes serializePreamble(const VaultPreamble &p);
  static VaultPreamble deserializePreamble(const RawBytes &data);

  static RawBytes serializeHeader(const VaultHeader &h);
  static VaultHeader deserializeHeader(const RawBytes &data);

  static RawBytes serializeEncryptedBlob(const EncryptedBlob &blob);
  static EncryptedBlob deserializeEncryptedBlob(const RawBytes &data);

  static RawBytes serializePageHeader(const PageHeader &h);
  static PageHeader deserializePageHeader(const RawBytes &data);

  static RawBytes serializeSlot(const Slot &s);
  static Slot deserializeSlot(const RawBytes &data);

  static RawBytes serializeSlottedLayout(const SlottedLayout &layout);
  static SlottedLayout deserializeSlottedLayout(const PageHeader &header,
                                                uint32_t pageSize,
                                                const RawBytes &data);

  static RawBytes serializeFreelistLayout(const FreelistLayout &layout);
  static FreelistLayout deserializeFreelistLayout(const PageHeader &header,
                                                  const RawBytes &data);

  static RawBytes serializeFreeLayout(const FreeLayout &);
  static FreeLayout deserializeFreeLayout(const PageHeader &header);

  static void writeString(RawBytes &out, const std::string &str);
  static std::string readString(const RawBytes &data, size_t &offset);

  static void writeUUID(RawBytes &out, const UUID &id);
  static UUID readUUID(const RawBytes &data, size_t &offset);

private:
  template <typename T> static void write(RawBytes &out, const T &value) {
    static_assert(std::is_trivially_copyable_v<T>,
                  "T must be trivially copyable");

    size_t old = out.size();
    out.resize(old + sizeof(T));

    std::memcpy(out.data() + old, &value, sizeof(T));
  }

  template <typename T> static T read(const RawBytes &data, size_t &offset) {
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

  template <typename T>
  static void readInto(const RawBytes &data, size_t &offset, T &out) {
    static_assert(std::is_trivially_copyable_v<T>,
                  "T must be trivially copyable");

    if (offset + sizeof(T) > data.size()) {
      throw std::runtime_error("Serialization buffer underflow");
    }

    std::memcpy(&out, data.data() + offset, sizeof(T));

    offset += sizeof(T);
  }
};
