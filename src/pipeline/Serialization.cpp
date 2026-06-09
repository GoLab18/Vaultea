#include "Serialization.h"
#include "storage/Constants.h"
#include "storage/VaultPreamble.h"
#include "storage/page/Constants.h"
#include "storage/page/PageLayout.h"
#include "util/Helpers.h"

#include <cstdint>
#include <cstring>

using namespace vault::util::time;

RawBytes Serialization::serializeEntry(const VaultEntry &e) {
  RawBytes out;

  writeUUID(out, e.id);
  writeString(out, e.name);
  writeUUID(out, e.folderId);

  write(out, e.createdAt);
  write(out, e.updatedAt);

  write(out, e.type);

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

VaultEntry Serialization::deserializeEntry(const RawBytes &data) {
  VaultEntry e;
  size_t offset = 0;

  e.id = readUUID(data, offset);
  e.name = readString(data, offset);
  e.folderId = readUUID(data, offset);

  e.createdAt = read<EpochTime>(data, offset);
  e.updatedAt = read<EpochTime>(data, offset);

  e.type = read<EntryType>(data, offset);

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

RawBytes Serialization::serializeFolder(const Folder &f) {
  RawBytes out;

  writeUUID(out, f.id);
  writeString(out, f.name);

  write(out, f.createdAt);
  write(out, f.updatedAt);

  return out;
}

Folder Serialization::deserializeFolder(const RawBytes &data) {
  Folder f;
  size_t offset = 0;

  f.id = readUUID(data, offset);
  f.name = readString(data, offset);

  f.createdAt = read<EpochTime>(data, offset);
  f.updatedAt = read<EpochTime>(data, offset);

  return f;
}

RawBytes Serialization::serializeIndexEntry(const IndexEntry &e) {
  RawBytes out;

  writeUUID(out, e.id);
  write(out, e.type);

  write(out, e.dataRef.pageId);
  write(out, e.dataRef.slotId);

  switch (e.type) {

  case IndexObjectType::Entry: {
    const auto &meta = std::get<ItemIndexMeta>(e.payload);
    writeUUID(out, meta.folderId);
    writeString(out, meta.name);

    break;
  }
  case IndexObjectType::Folder: {
    const auto &meta = std::get<FolderIndexMeta>(e.payload);
    writeString(out, meta.name);

    break;
  }
  }

  return out;
}

IndexEntry Serialization::deserializeIndexEntry(const RawBytes &data) {
  IndexEntry e;
  size_t offset = 0;

  e.id = readUUID(data, offset);
  e.type = read<IndexObjectType>(data, offset);

  e.dataRef = {read<PageId>(data, offset), read<SlotId>(data, offset)};

  switch (e.type) {

  case IndexObjectType::Entry: {
    ItemIndexMeta meta;
    meta.folderId = readUUID(data, offset);
    meta.name = readString(data, offset);

    e.payload = meta;

    break;
  }

  case IndexObjectType::Folder: {
    FolderIndexMeta meta;
    meta.name = readString(data, offset);

    e.payload = meta;

    break;
  }

  default:
    break;
  }

  return e;
}

RawBytes Serialization::serializePreamble(const VaultPreamble &p) {
  RawBytes out;

  write(out, p.magic);
  write(out, p.version);

  write(out, p.uuid);
  write(out, p.salt);
  write(out, p.keyCheck);

  write(out, p.pageSize);
  write(out, p.headerSize);

  return out;
}

VaultPreamble Serialization::deserializePreamble(const RawBytes &data) {
  VaultPreamble p;
  size_t offset = 0;

  readInto(data, offset, p.magic);

  p.version = read<uint32_t>(data, offset);

  readInto(data, offset, p.uuid);
  readInto(data, offset, p.salt);
  readInto(data, offset, p.keyCheck);

  p.pageSize = read<uint32_t>(data, offset);
  p.headerSize = read<uint32_t>(data, offset);

  return p;
}

RawBytes Serialization::serializeHeader(const VaultHeader &h) {
  RawBytes out;

  write(out, h.indexRootPage);
  write(out, h.dataRootPage);
  write(out, h.freelistRootPage);

  write(out, h.entryCount);
  write(out, h.folderCount);

  write(out, h.createdAt);
  write(out, h.updatedAt);

  return out;
}

VaultHeader Serialization::deserializeHeader(const RawBytes &data) {
  VaultHeader h;
  size_t offset = 0;

  h.indexRootPage = read<PageId>(data, offset);
  h.dataRootPage = read<PageId>(data, offset);
  h.freelistRootPage = read<PageId>(data, offset);

  h.entryCount = read<uint64_t>(data, offset);
  h.folderCount = read<uint64_t>(data, offset);

  h.createdAt = read<EpochTime>(data, offset);
  h.updatedAt = read<EpochTime>(data, offset);

  return h;
}

RawBytes Serialization::serializeProcessedBlob(const ProcessedBlob &blob) {
  RawBytes out;

  write(out, blob.compressionType);
  write(out, blob.originalSize);

  write(out, blob.nonce);

  uint32_t size = static_cast<uint32_t>(blob.ciphertext.size());
  write(out, size);

  out.insert(out.end(), blob.ciphertext.begin(), blob.ciphertext.end());

  return out;
}

ProcessedBlob Serialization::deserializeProcessedBlob(const RawBytes &data) {
  size_t offset = 0;

  ProcessedBlob blob;

  blob.compressionType = read<CompressionType>(data, offset);
  blob.originalSize = read<uint32_t>(data, offset);

  readInto(data, offset, blob.nonce);

  uint32_t size = read<uint32_t>(data, offset);

  if (offset + size > data.size()) {
    throw std::runtime_error("Processed blob corrupted or truncated");
  }

  blob.ciphertext.resize(size);

  std::memcpy(blob.ciphertext.data(), data.data() + offset, size);

  return blob;
}

RawBytes Serialization::serializePageHeader(const PageHeader &h) {
  RawBytes out;

  write(out, h.pageId);
  write(out, h.type);

  write(out, h.nextPage);
  write(out, h.prevPage);

  return out;
}

PageHeader Serialization::deserializePageHeader(const RawBytes &data) {
  PageHeader h;

  size_t offset = 0;

  h.pageId = read<PageId>(data, offset);
  h.type = read<PageType>(data, offset);

  h.nextPage = read<PageId>(data, offset);
  h.prevPage = read<PageId>(data, offset);

  return h;
}

RawBytes Serialization::serializeSlottedLayout(const SlottedLayout &layout) {
  RawBytes out;

  write(out, layout.lower);
  write(out, layout.upper);

  write(out, layout.pageSlots.size());

  for (const auto &s : layout.pageSlots) {
    write(out, s.offset);
    write(out, s.size);
    write(out, s.state);
  }

  return out;
}

SlottedLayout Serialization::deserializeSlottedLayout(const PageHeader &header,
                                                      uint32_t pageSize,
                                                      const RawBytes &data) {

  size_t offset = HEADER_SIZE;

  SlottedLayout layout(header, pageSize);

  layout.lower = read<uint16_t>(data, offset);
  layout.upper = read<uint16_t>(data, offset);

  uint16_t slotCount = read<uint16_t>(data, offset);
  layout.pageSlots.reserve(slotCount);

  for (uint16_t i = 0; i < slotCount; i++) {
    Slot s;
    s.offset = read<uint16_t>(data, offset);
    s.size = read<uint16_t>(data, offset);
    s.state = read<SlotState>(data, offset);
    layout.pageSlots.push_back(s);
  }

  return layout;
}

RawBytes Serialization::serializeFreelistLayout(const FreelistLayout &layout) {
  RawBytes out;

  uint16_t count = static_cast<uint16_t>(layout.freePages.size());
  write(out, count);

  for (PageId id : layout.freePages) {
    write(out, id);
  }

  return out;
}

FreelistLayout
Serialization::deserializeFreelistLayout(const PageHeader &header,
                                         const RawBytes &data) {

  size_t offset = HEADER_SIZE;

  FreelistLayout layout(header);

  uint16_t count = read<uint16_t>(data, offset);
  layout.freePages.reserve(count);

  for (uint16_t i = 0; i < count; i++) {
    layout.freePages.push_back(read<PageId>(data, offset));
  }

  return layout;
}

RawBytes Serialization::serializeFreeLayout(const FreeLayout &) { return {}; }

FreeLayout Serialization::deserializeFreeLayout(const PageHeader &header) {
  return FreeLayout(header);
}

void Serialization::writeString(RawBytes &out, const std::string &str) {
  uint32_t size = static_cast<uint32_t>(str.size());
  write(out, size);

  size_t old = out.size();
  out.resize(old + size);

  std::memcpy(out.data() + old, str.data(), size);
}

std::string Serialization::readString(const RawBytes &data, size_t &offset) {
  uint32_t size = read<uint32_t>(data, offset);

  if (offset + size > data.size()) {
    throw std::runtime_error("String exceeds serialization buffer");
  }

  std::string out(reinterpret_cast<const char *>(data.data() + offset), size);

  offset += size;

  return out;
}

void Serialization::writeUUID(RawBytes &out, const UUID &id) {
  const auto &raw = id.raw();
  out.insert(out.end(), raw.begin(), raw.end());
}

UUID Serialization::readUUID(const RawBytes &data, size_t &offset) {
  if (offset + vault::storage::UUID_SIZE > data.size()) {
    throw std::runtime_error("UUID out of bounds");
  }

  std::array<uint8_t, vault::storage::UUID_SIZE> buf{};

  std::memcpy(buf.data(), data.data() + offset, buf.size());
  offset += buf.size();

  return UUID::fromRaw(buf);
}
