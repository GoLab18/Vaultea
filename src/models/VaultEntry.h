#pragma once

#include "BaseVaultObject.h"
#include "storage/UUID.h"

#include <string>
#include <variant>
#include <vector>

enum class CardType : uint8_t { Visa, MasterCard, Amex, Discover, Other };
enum class Sex : uint8_t { Male, Female, Other, Unknown };

struct LoginData {
  std::string username;
  std::string password;
  std::vector<std::string> urls;
};

struct CardData {
  CardType type;
  std::string holder;
  std::string number;
  std::string expiry;
  std::string cvv;
};

struct IdentityData {
  Sex sex;

  // Personal Details
  std::string firstName;
  std::string middleName;
  std::string lastName;
  std::string username;
  std::string company;

  // Identification
  std::string ssn;
  std::string passportNumber;
  std::string licenseNumber;

  // Contact Info
  std::string email;
  std::string phone;

  // Address
  std::string city;
  std::string state;
  std::string zip;
  std::string country;
};

struct SecureNoteData {
  std::string content;
};

enum class EntryType : uint8_t { Login, Card, Identity, Note };

using EntryData =
    std::variant<LoginData, CardData, IdentityData, SecureNoteData>;

struct VaultEntry : public BaseVaultObject {
  std::string name;
  std::string notes;

  UUID folderId;
  EntryType type;
  EntryData data;
};
