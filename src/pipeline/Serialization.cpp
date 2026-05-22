#include "Serialization.h"
#include <cstring>
#include <vector>

std::vector<uint8_t> Serialization::serializeEntry(const VaultEntry &e) {
  std::vector<uint8_t> out;

  writeString(out, e.id);
  writeString(out, e.name);
  writeString(out, e.folderId);

  write(out, e.createdAt);
  write(out, e.updatedAt);

  uint32_t type = static_cast<uint32_t>(e.type);
  write(out, type);

  switch (e.type) {
  case EntryType::Login: {
    auto d = std::get<LoginData>(e.data);
    writeString(out, d.username);
    writeString(out, d.password);

    uint32_t n = d.urls.size();
    write(out, n);
    for (auto &u : d.urls)
      writeString(out, u);
    break;
  }

  case EntryType::Card: {
    auto d = std::get<CardData>(e.data);
    writeString(out, d.holder);
    writeString(out, d.number);
    writeString(out, d.expiry);
    writeString(out, d.cvv);
    break;
  }

  case EntryType::Identity: {
    auto d = std::get<IdentityData>(e.data);
    writeString(out, d.fullName);
    writeString(out, d.email);
    writeString(out, d.phone);
    break;
  }

  case EntryType::Note: {
    auto d = std::get<SecureNoteData>(e.data);
    writeString(out, d.content);
    break;
  }
  }

  return out;
}

VaultEntry Serialization::deserializeEntry(const std::vector<uint8_t> &data) {
  VaultEntry e;
  size_t offset = 0;

  e.id = readString(data, offset);
  e.name = readString(data, offset);
  e.folderId = readString(data, offset);

  e.createdAt = read<uint64_t>(data, offset);
  e.updatedAt = read<uint64_t>(data, offset);

  uint32_t type = read<uint32_t>(data, offset);
  e.type = static_cast<EntryType>(type);

  switch (e.type) {
  case EntryType::Login: {
    LoginData d;

    d.username = readString(data, offset);
    d.password = readString(data, offset);

    uint32_t n = read<uint32_t>(data, offset);
    d.urls.reserve(n);

    for (uint32_t i = 0; i < n; i++)
      d.urls.push_back(readString(data, offset));

    e.data = d;
    break;
  }

  case EntryType::Card: {
    CardData d;

    d.holder = readString(data, offset);
    d.number = readString(data, offset);
    d.expiry = readString(data, offset);
    d.cvv = readString(data, offset);

    e.data = d;
    break;
  }

  case EntryType::Identity: {
    IdentityData d;

    d.fullName = readString(data, offset);
    d.email = readString(data, offset);
    d.phone = readString(data, offset);

    e.data = d;
    break;
  }

  case EntryType::Note: {
    SecureNoteData d;

    d.content = readString(data, offset);

    e.data = d;
    break;
  }
  }

  return e;
}

std::vector<uint8_t> Serialization::serializeFolder(const Folder &f) {
  std::vector<uint8_t> out;

  writeString(out, f.id);
  writeString(out, f.name);

  write(out, f.createdAt);
  write(out, f.updatedAt);

  return out;
}

Folder Serialization::deserializeFolder(const std::vector<uint8_t> &data) {
  Folder f;
  size_t offset = 0;

  f.id = readString(data, offset);
  f.name = readString(data, offset);

  f.createdAt = read<uint64_t>(data, offset);
  f.updatedAt = read<uint64_t>(data, offset);

  return f;
}

std::vector<uint8_t> Serialization::serializeIndex(const BTreeIndex &index) {
  std::vector<uint8_t> out;

  auto allEntries = index.all();

  uint32_t count = static_cast<uint32_t>(allEntries.size());
  write(out, count);

  for (const auto &e : allEntries) {
    writeString(out, e.id);
    writeString(out, e.name);
    writeString(out, e.folderId);

    write(out, e.offset);
    write(out, e.size);

    uint8_t compressed = static_cast<uint8_t>(e.compressed);
    write(out, compressed);
  }

  return out;
}

std::vector<IndexEntry>
Serialization::deserializeIndex(const std::vector<uint8_t> &data) {
  std::vector<IndexEntry> out;
  size_t offset = 0;

  uint32_t count = read<uint32_t>(data, offset);

  for (uint32_t i = 0; i < count; i++) {
    IndexEntry e;

    e.id = readString(data, offset);
    e.name = readString(data, offset);
    e.folderId = readString(data, offset);

    e.offset = read<uint64_t>(data, offset);
    e.size = read<uint32_t>(data, offset);

    uint8_t compressed = read<uint8_t>(data, offset);
    e.compressed = static_cast<bool>(compressed);

    out.push_back(e);
  }

  return out;
}

std::vector<uint8_t> Serialization::serializePreamble(const VaultPreamble &p) {
  std::vector<uint8_t> out;

  write(out, p.magic);
  write(out, p.version);

  write(out, p.uuid);
  write(out, p.salt);

  write(out, p.headerOffset);
  write(out, p.headerSize);

  write(out, p.indexOffset);
  write(out, p.indexSize);

  write(out, p.dataOffset);

  return out;
}

VaultPreamble
Serialization::deserializePreamble(const std::vector<uint8_t> &data) {
  VaultPreamble p{};
  size_t offset = 0;

  readInto(data, offset, p.magic);

  p.version = read<uint32_t>(data, offset);

  readInto(data, offset, p.uuid);
  readInto(data, offset, p.salt);

  p.headerOffset = read<uint64_t>(data, offset);
  p.headerSize = read<uint64_t>(data, offset);

  p.indexOffset = read<uint64_t>(data, offset);
  p.indexSize = read<uint64_t>(data, offset);

  p.dataOffset = read<uint64_t>(data, offset);

  return p;
}

std::vector<uint8_t> Serialization::serializeHeader(const VaultHeader &h) {
  std::vector<uint8_t> out;

  write(out, h.createdAt);
  write(out, h.updatedAt);
  write(out, h.entryCount);
  write(out, h.folderCount);

  return out;
}

VaultHeader Serialization::deserializeHeader(const std::vector<uint8_t> &data) {
  VaultHeader h;
  size_t offset = 0;

  h.createdAt = read<uint64_t>(data, offset);
  h.updatedAt = read<uint64_t>(data, offset);
  h.entryCount = read<uint64_t>(data, offset);
  h.folderCount = read<uint64_t>(data, offset);

  return h;
}
