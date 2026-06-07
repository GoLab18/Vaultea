#include "VaultEngine.h"
#include "pipeline/Serialization.h"
#include "storage/Constants.h"
#include "util/Helpers.h"

#include <stdexcept>

using namespace vault::storage;

void VaultEngine::validateOpened() const {
  if (!opened)
    throw std::runtime_error("Vault not opened");
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
  preamble.salt = crypto.generateSalt();
  preamble.pageSize = page::DEFAULT_PAGE_SIZE;

  auto key = crypto.deriveMasterKey(password, preamble.salt);

  // Don't they require allocation (?)
  header.indexRootPage = page::INVALID_PAGE;
  header.dataRootPage = page::INVALID_PAGE;

  // TODO lazy assignment -> assigned only on first free page added (?)
  // Don't know how to handle that properly though
  header.freelistRootPage = page::INVALID_PAGE;

  header.entryCount = 0;
  header.folderCount = 0;

  header.createdAt = vault::util::time::now();
  header.updatedAt = header.createdAt;

  storage.write(0, Serialization::serializePreamble(preamble).data(),
                VAULT_PREAMBLE_SIZE);

  persistHeader();

  pager = std::make_unique<Pager>(storage, preamble.pageSize,
                                  VAULT_PREAMBLE_SIZE + VAULT_HEADER_SIZE,
                                  header.freelistRootPage);

  dataManager = std::make_unique<DataManager>(*pager, header.dataRootPage);
  indexManager = std::make_unique<IndexManager>(*pager, header.indexRootPage);
}

bool VaultEngine::openVault(const std::string &path,
                            const std::string &password) {
  this->path = path;
  this->password = password;

  if (!storage.open(path))
    return false;

  loadOrInitHeader();

  pager = std::make_unique<Pager>(storage, preamble.pageSize,
                                  VAULT_PREAMBLE_SIZE + VAULT_HEADER_SIZE,
                                  header.freelistRootPage);

  dataManager = std::make_unique<DataManager>(*pager, header.dataRootPage);
  indexManager = std::make_unique<IndexManager>(*pager, header.indexRootPage);

  opened = true;
  return true;
}

void VaultEngine::loadOrInitHeader() {
  auto preambleBytes = storage.read(0, VAULT_PREAMBLE_SIZE);
  preamble = Serialization::deserializePreamble(preambleBytes);

  auto headerBytes = storage.read(VAULT_PREAMBLE_SIZE, VAULT_HEADER_SIZE);
  header = Serialization::deserializeHeader(headerBytes);
}

void VaultEngine::closeVault() {
  if (!opened)
    return;

  persistHeader();

  pager->flush();
  storage.close();

  opened = false;
}

void VaultEngine::persistHeader() {
  storage.write(VAULT_PREAMBLE_SIZE,
                Serialization::serializeHeader(header).data(),
                VAULT_HEADER_SIZE);
}

std::string VaultEngine::addEntry(const VaultEntry &entry) {
  validateOpened();

  auto dataBytes = Serialization::serializeEntry(entry);

  auto ref = dataManager->insert(dataBytes);

  IndexEntry indexEntry{
      .id = entry.id,
      .type = IndexObjectType::Entry,
      .dataRef = ref,
      .compressed = false,
      .payload = ItemIndexMeta{.folderId = entry.id, .name = entry.name}};

  indexManager->insert(indexEntry);

  header.entryCount++;
  header.updatedAt = vault::util::time::now();

  persistHeader();

  return entry.id.toString();
  ;
}

std::optional<VaultEntry> VaultEngine::getEntry(const std::string &idStr) {
  validateOpened();

  auto id = UUID::fromString(idStr);

  auto *index = indexManager->findEntry(id);
  if (!index)
    return std::nullopt;

  auto bytes = dataManager->read(index->dataRef);

  return Serialization::deserializeEntry(bytes);
}

bool VaultEngine::updateEntry(const VaultEntry &entry) {
  validateOpened();

  auto *existing = indexManager->findEntry(entry.id);
  if (!existing)
    return false;

  auto bytes = Serialization::serializeEntry(entry);

  auto moved = dataManager->update(existing->dataRef, bytes);

  IndexEntry updated = *existing;
  updated.dataRef = moved ? *moved : existing->dataRef;
  updated.payload =
      ItemIndexMeta{.folderId = entry.folderId, .name = entry.name};

  indexManager->update(updated);

  header.updatedAt = vault::util::time::now();

  persistHeader();

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

  persistHeader();

  return true;
}

std::vector<VaultEntry> VaultEngine::getAllEntries() {
  validateOpened();

  auto entries = indexManager->scanAll();

  std::vector<VaultEntry> out;
  out.reserve(entries.size());

  for (const auto &e : entries) {
    if (e.type != IndexObjectType::Entry)
      continue;

    auto bytes = dataManager->read(e.dataRef);
    out.push_back(Serialization::deserializeEntry(bytes));
  }

  return out;
}

std::vector<VaultEntry>
VaultEngine::getByFolder(const std::string &folderIdStr) {
  validateOpened();

  auto folderId = UUID::fromString(folderIdStr);

  auto entries = indexManager->findByFolder(folderId);

  std::vector<VaultEntry> out;
  out.reserve(entries.size());

  for (const auto &e : entries) {
    auto bytes = dataManager->read(e.dataRef);
    out.push_back(Serialization::deserializeEntry(bytes));
  }

  return out;
}

std::vector<VaultEntry> VaultEngine::searchEntries(const std::string &query) {
  validateOpened();

  auto entries = indexManager->findEntriesByName(query);

  std::vector<VaultEntry> out;
  out.reserve(entries.size());

  for (const auto &e : entries) {
    auto bytes = dataManager->read(e.dataRef);
    out.push_back(Serialization::deserializeEntry(bytes));
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

  auto bytes = Serialization::serializeFolder(folder);

  auto ref = dataManager->insert(bytes);

  IndexEntry indexEntry{.id = folder.id,
                        .type = IndexObjectType::Folder,
                        .dataRef = ref,
                        .compressed = false,
                        .payload = FolderIndexMeta{.name = folder.name}};

  indexManager->insert(indexEntry);

  header.folderCount++;
  header.updatedAt = vault::util::time::now();

  persistHeader();

  return folder.id.toString();
}

bool VaultEngine::renameFolder(const std::string &idStr,
                               const std::string &newName) {
  validateOpened();

  auto id = UUID::fromString(idStr);

  auto *existing = indexManager->findFolder(id);
  if (!existing)
    return false;

  auto bytes = dataManager->read(existing->dataRef);

  Folder folder = Serialization::deserializeFolder(bytes);
  folder.name = newName;

  auto newBytes = Serialization::serializeFolder(folder);

  auto moved = dataManager->update(existing->dataRef, newBytes);

  IndexEntry updated = *existing;
  updated.dataRef = moved ? *moved : existing->dataRef;
  updated.payload = FolderIndexMeta{.name = newName};

  indexManager->update(updated);

  header.updatedAt = vault::util::time::now();

  persistHeader();

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

  persistHeader();

  return true;
}

std::vector<Folder> VaultEngine::getFolders() {
  validateOpened();

  auto entries = indexManager->scanAll();

  std::vector<Folder> out;
  out.reserve(entries.size());

  for (const auto &e : entries) {
    if (e.type != IndexObjectType::Folder)
      continue;

    auto bytes = dataManager->read(e.dataRef);
    out.push_back(Serialization::deserializeFolder(bytes));
  }

  return out;
}

std::vector<Folder> VaultEngine::searchFolders(const std::string &query) {
  validateOpened();

  auto folders = indexManager->findFoldersByName(query);

  std::vector<Folder> out;
  out.reserve(folders.size());

  for (const auto &f : folders) {
    auto bytes = dataManager->read(f.dataRef);
    out.push_back(Serialization::deserializeFolder(bytes));
  }

  return out;
}

uint64_t VaultEngine::entryCount() const { return header.entryCount; }

uint64_t VaultEngine::folderCount() const { return header.folderCount; }
