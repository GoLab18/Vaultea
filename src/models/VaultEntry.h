#pragma once

#include "BaseVaultObject.h"
#include "storage/UUID.h"

#include <string>
#include <variant>
#include <vector>

struct LoginData {
  std::string username;
  std::string password;
  std::vector<std::string> urls;
};

struct CardData {
  std::string holder;
  std::string number;
  std::string expiry;
  std::string cvv;
};

struct IdentityData {
  std::string fullName;
  std::string email;
  std::string phone;
};

struct SecureNoteData {
  std::string content;
};

enum class EntryType { Login, Card, Identity, Note };

using EntryData =
    std::variant<LoginData, CardData, IdentityData, SecureNoteData>;

struct VaultEntry : public BaseVaultObject {
  std::string name;

  UUID folderId;

  EntryType type;

  EntryData data;
};
