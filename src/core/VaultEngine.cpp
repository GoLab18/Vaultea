#include "VaultEngine.h"
#include "core/VaultLoader.h"
#include "crypto/CryptoService.h"
#include "pipeline/DefaultCodec.h"
#include "pipeline/LZ4Compressor.h"
#include "pipeline/Serialization.h"
#include "storage/Constants.h"
#include "util/Helpers.h"

#include <stdexcept>

using namespace vault::storage;

void VaultEngine::validateOpened() const {
  if (!opened)
    throw std::runtime_error("Vault not opened");
}

bool VaultEngine::isOpened() const { return opened; }

void VaultEngine::loadRootPages() {
  header.dataRootPage = dataManager->getRootPage();
  header.indexRootPage = indexManager->getRootPage();
  header.freelistRootPage = pager->getFreelistRootPage();
}

void VaultEngine::commit() {
  pager->flush();

  loadRootPages();

  persistHeader();
}

void VaultEngine::persistPreamble() {
  storage.write(0, Serialization::serializePreamble(preamble).data(),
                VAULT_PREAMBLE_SIZE);
}

bool VaultEngine::createVault(const std::string &path,
                              const std::string &password) {
  this->path = path;
  this->password = password;

  if (!storage.create(path))
    return false;

  initNewVault();

  opened = true;
  return true;
}

void VaultEngine::initNewVault() {
  preamble.magic = MAGIC;
  preamble.version = VERSION;
  preamble.uuid = UUID::random();

  preamble.salt = CryptoService::generateSalt();
  masterKey = CryptoService::deriveMasterKey(password, preamble.salt);

  preamble.keyCheck = CryptoService::deriveKeyCheck(masterKey);
  preamble.pageSize = page::DEFAULT_PAGE_SIZE;

  header.indexRootPage = page::INVALID_PAGE;
  header.dataRootPage = page::INVALID_PAGE;
  header.freelistRootPage = page::INVALID_PAGE;

  header.entryCount = 0;
  header.folderCount = 0;

  header.createdAt = vault::util::time::now();
  header.updatedAt = header.createdAt;

  persistPreamble();

  compressor = std::make_shared<LZ4Compressor>();
  codec = std::make_unique<DefaultCodec>(masterKey, compressor);

  RawBytes encodedHeader;
  processHeader(encodedHeader);
  preamble.headerSize = encodedHeader.size();

  pager = std::make_unique<Pager>(storage, preamble.pageSize,
                                  VAULT_PREAMBLE_SIZE + preamble.headerSize,
                                  header.freelistRootPage);

  dataManager = std::make_unique<DataManager>(*pager, header.dataRootPage);

  auto loadedIndexEntries =
      VaultLoader::loadIndex(*pager, header.indexRootPage, *codec);
  indexManager = std::make_unique<IndexManager>(*pager, header.indexRootPage,
                                                loadedIndexEntries);

  commit();
}

bool VaultEngine::openVault(const std::string &path,
                            const std::string &password) {
  this->path = path;
  this->password = password;

  if (!storage.open(path))
    return false;

  auto preambleBytes = storage.read(0, VAULT_PREAMBLE_SIZE);
  preamble = Serialization::deserializePreamble(preambleBytes);

  masterKey = CryptoService::deriveMasterKey(password, preamble.salt);

  if (!verifyPassword())
    return false;

  compressor = std::make_shared<LZ4Compressor>();
  codec = std::make_unique<DefaultCodec>(masterKey, compressor);

  auto headerBytes = storage.read(VAULT_PREAMBLE_SIZE, preamble.headerSize);
  unprocessHeader(headerBytes);

  pager = std::make_unique<Pager>(storage, preamble.pageSize,
                                  VAULT_PREAMBLE_SIZE + preamble.headerSize,
                                  header.freelistRootPage);

  dataManager = std::make_unique<DataManager>(*pager, header.dataRootPage);

  auto loadedIndexEntries =
      VaultLoader::loadIndex(*pager, header.indexRootPage, *codec);
  indexManager = std::make_unique<IndexManager>(*pager, header.indexRootPage,
                                                loadedIndexEntries);

  loadRootPages();

  opened = true;
  return true;
}

void VaultEngine::processHeader(RawBytes &out) const {
  auto plain = Serialization::serializeHeader(header);
  out = codec->encodeHeader(plain);
}

void VaultEngine::unprocessHeader(const RawBytes &in) {
  auto plain = codec->decodeHeader(in);
  header = Serialization::deserializeHeader(plain);
}

void VaultEngine::persistHeader() {
  RawBytes encrypted;
  processHeader(encrypted);

  storage.write(VAULT_PREAMBLE_SIZE, encrypted.data(), encrypted.size());

  storage.flush();
}

void VaultEngine::closeVault() {
  if (!opened)
    return;

  commit();

  storage.close();

  opened = false;
}

bool VaultEngine::verifyPassword() {
  const auto computed = CryptoService::deriveKeyCheck(masterKey);
  return computed == preamble.keyCheck;
}

std::string VaultEngine::addEntry(const VaultEntry &entry) {
  validateOpened();

  auto plain = Serialization::serializeEntry(entry);
  auto encoded = codec->encodeData(plain);

  auto ref = dataManager->insert(encoded);

  IndexEntry indexEntry{
      .id = entry.id,
      .type = IndexObjectType::Entry,
      .dataRef = ref,
      .payload = ItemIndexMeta{.folderId = entry.folderId, .name = entry.name}};

  auto indexPlain = Serialization::serializeIndexEntry(indexEntry);
  auto indexEncoded = codec->encodeIndex(indexPlain);

  indexManager->insert(indexEntry, indexEncoded);

  header.entryCount++;
  header.updatedAt = vault::util::time::now();

  commit();

  return entry.id.toString();
}

std::optional<VaultEntry> VaultEngine::getEntry(const std::string &idStr) {
  validateOpened();

  auto id = UUID::fromString(idStr);

  auto *index = indexManager->findEntry(id);
  if (!index)
    return std::nullopt;

  auto encoded = dataManager->read(index->dataRef);
  auto plain = codec->decodeData(encoded);

  return Serialization::deserializeEntry(plain);
}

bool VaultEngine::updateEntry(const VaultEntry &entry) {
  validateOpened();

  auto *existing = indexManager->findEntry(entry.id);
  if (!existing)
    return false;

  auto plain = Serialization::serializeEntry(entry);
  auto encoded = codec->encodeData(plain);

  auto moved = dataManager->update(existing->dataRef, encoded);

  IndexEntry updated = *existing;
  updated.dataRef = moved ? *moved : existing->dataRef;
  updated.payload =
      ItemIndexMeta{.folderId = entry.folderId, .name = entry.name};

  auto indexPlain = Serialization::serializeIndexEntry(updated);
  auto indexEncoded = codec->encodeIndex(indexPlain);

  indexManager->update(updated, indexEncoded);

  header.updatedAt = vault::util::time::now();

  commit();

  return true;
}

bool VaultEngine::deleteEntry(const std::string &idStr) {
  validateOpened();

  auto id = UUID::fromString(idStr);

  auto *existing = indexManager->findEntry(id);
  if (!existing)
    return false;

  dataManager->remove(existing->dataRef);
  indexManager->remove(id);

  header.entryCount--;
  header.updatedAt = vault::util::time::now();

  commit();

  return true;
}

std::vector<VaultEntry> VaultEngine::getAllEntries() {
  validateOpened();

  auto entries = indexManager->scanAll();

  std::vector<VaultEntry> out;

  for (const auto &e : entries) {
    if (e.type != IndexObjectType::Entry)
      continue;

    auto encoded = dataManager->read(e.dataRef);
    auto plain = codec->decodeData(encoded);

    out.push_back(Serialization::deserializeEntry(plain));
  }

  return out;
}

std::vector<VaultEntry>
VaultEngine::getByFolder(const std::string &folderIdStr) {
  validateOpened();

  auto folderId = UUID::fromString(folderIdStr);
  auto entries = indexManager->findByFolder(folderId);

  std::vector<VaultEntry> out;

  for (const auto &e : entries) {
    auto encoded = dataManager->read(e.dataRef);
    auto plain = codec->decodeData(encoded);

    out.push_back(Serialization::deserializeEntry(plain));
  }

  return out;
}

std::vector<VaultEntry> VaultEngine::searchEntries(const std::string &query) {
  validateOpened();

  auto entries = indexManager->findEntriesByName(query);

  std::vector<VaultEntry> out;

  for (const auto &e : entries) {
    auto encoded = dataManager->read(e.dataRef);
    auto plain = codec->decodeData(encoded);

    out.push_back(Serialization::deserializeEntry(plain));
  }

  return out;
}

std::string VaultEngine::createFolder(const std::string &name) {
  validateOpened();

  Folder folder;
  folder.id = UUID::random();
  folder.name = name;
  folder.createdAt = vault::util::time::now();
  folder.updatedAt = folder.createdAt;

  auto plain = Serialization::serializeFolder(folder);
  auto encoded = codec->encodeData(plain);

  auto ref = dataManager->insert(encoded);

  IndexEntry indexEntry{.id = folder.id,
                        .type = IndexObjectType::Folder,
                        .dataRef = ref,
                        .payload = FolderIndexMeta{.name = folder.name}};

  auto indexPlain = Serialization::serializeIndexEntry(indexEntry);
  auto indexEncoded = codec->encodeIndex(indexPlain);

  indexManager->insert(indexEntry, indexEncoded);

  header.folderCount++;
  header.updatedAt = vault::util::time::now();

  commit();

  return folder.id.toString();
}

bool VaultEngine::renameFolder(const std::string &idStr,
                               const std::string &newName) {
  validateOpened();

  auto id = UUID::fromString(idStr);

  auto *existing = indexManager->findFolder(id);
  if (!existing)
    return false;

  auto encoded = dataManager->read(existing->dataRef);
  auto plain = codec->decodeData(encoded);

  Folder folder = Serialization::deserializeFolder(plain);
  folder.name = newName;
  folder.updatedAt = vault::util::time::now();

  auto newPlain = Serialization::serializeFolder(folder);
  auto newEncoded = codec->encodeData(newPlain);

  auto moved = dataManager->update(existing->dataRef, newEncoded);

  IndexEntry updated = *existing;
  updated.dataRef = moved ? *moved : existing->dataRef;
  updated.payload = FolderIndexMeta{.name = newName};

  auto indexPlain = Serialization::serializeIndexEntry(updated);
  auto indexEncoded = codec->encodeIndex(indexPlain);

  indexManager->update(updated, indexEncoded);

  header.updatedAt = vault::util::time::now();

  commit();

  return true;
}

bool VaultEngine::deleteFolder(const std::string &idStr) {
  validateOpened();

  auto id = UUID::fromString(idStr);

  auto *existing = indexManager->findFolder(id);
  if (!existing)
    return false;

  auto children = indexManager->findByFolder(id);
  if (!children.empty())
    return false;

  dataManager->remove(existing->dataRef);
  indexManager->remove(id);

  header.folderCount--;
  header.updatedAt = vault::util::time::now();

  commit();

  return true;
}

std::vector<Folder> VaultEngine::getFolders() {
  validateOpened();

  auto entries = indexManager->scanAll();

  std::vector<Folder> out;

  for (const auto &e : entries) {
    if (e.type != IndexObjectType::Folder)
      continue;

    auto encoded = dataManager->read(e.dataRef);
    auto plain = codec->decodeData(encoded);

    out.push_back(Serialization::deserializeFolder(plain));
  }

  return out;
}

std::vector<Folder> VaultEngine::searchFolders(const std::string &query) {
  validateOpened();

  auto folders = indexManager->findFoldersByName(query);

  std::vector<Folder> out;

  for (const auto &f : folders) {
    auto encoded = dataManager->read(f.dataRef);
    auto plain = codec->decodeData(encoded);

    out.push_back(Serialization::deserializeFolder(plain));
  }

  return out;
}

bool VaultEngine::addToFolder(const std::string &entryIdStr,
                              const std::string &folderIdStr) {
  validateOpened();

  auto entryId = UUID::fromString(entryIdStr);
  auto folderId = UUID::fromString(folderIdStr);

  auto *entry = indexManager->findEntry(entryId);
  if (!entry)
    return false;

  auto *folder = indexManager->findFolder(folderId);
  if (!folder)
    return false;

  auto encoded = dataManager->read(entry->dataRef);
  auto plain = codec->decodeData(encoded);

  VaultEntry v = Serialization::deserializeEntry(plain);
  v.folderId = folderId;

  auto updatedPlain = Serialization::serializeEntry(v);
  auto updatedEncoded = codec->encodeData(updatedPlain);

  auto moved = dataManager->update(entry->dataRef, updatedEncoded);

  IndexEntry updated = *entry;
  updated.dataRef = moved ? *moved : entry->dataRef;
  updated.payload = ItemIndexMeta{.folderId = folderId, .name = v.name};

  auto indexPlain = Serialization::serializeIndexEntry(updated);
  auto indexEncoded = codec->encodeIndex(indexPlain);

  indexManager->update(updated, indexEncoded);

  header.updatedAt = vault::util::time::now();

  commit();

  return true;
}

uint64_t VaultEngine::entryCount() const { return header.entryCount; }

uint64_t VaultEngine::folderCount() const { return header.folderCount; }
