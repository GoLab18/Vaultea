#pragma once

#include "models/RecordRef.h"
#include "storage/UUID.h"

#include <cstdint>
#include <string>
#include <variant>

enum class IndexObjectType : uint8_t { Entry, Folder };

struct ItemIndexMeta {
  UUID folderId;
  std::string name;
};

struct FolderIndexMeta {
  std::string name;
};

using IndexPayload = std::variant<ItemIndexMeta, FolderIndexMeta>;

struct IndexEntry {
  UUID id;

  IndexObjectType type;

  RecordRef dataRef;

  bool compressed;

  IndexPayload payload;
};

struct LoadedIndexEntry {
  IndexEntry entry;
  RecordRef indexRef;
};
