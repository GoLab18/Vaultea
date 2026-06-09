#pragma once

#include "crypto/Constants.h"
#include "managers/DataManager.h"
#include "managers/IndexManager.h"
#include "models/Folder.h"
#include "models/VaultEntry.h"
#include "pipeline/Codec.h"
#include "pipeline/ICompressor.h"
#include "storage/StorageEngine.h"
#include "storage/VaultHeader.h"
#include "storage/VaultPreamble.h"
#include "storage/page/Pager.h"

#include <memory>
#include <string>

class VaultEngine {
public:
  bool createVault(const std::string &path, const std::string &password);
  bool openVault(const std::string &path, const std::string &password);
  void closeVault();

  std::string addEntry(const VaultEntry &entry);
  bool updateEntry(const VaultEntry &entry);
  bool deleteEntry(const std::string &id);
  std::optional<VaultEntry> getEntry(const std::string &id);

  std::vector<VaultEntry> getAllEntries();
  std::vector<VaultEntry> getByFolder(const std::string &folderId);

  std::vector<VaultEntry> searchEntries(const std::string &query);

  std::string createFolder(const std::string &name);
  bool renameFolder(const std::string &id, const std::string &newName);
  bool deleteFolder(const std::string &id);
  std::vector<Folder> getFolders();
  std::vector<Folder> searchFolders(const std::string &query);

  bool addToFolder(const std::string &entryId, const std::string &folderId);

  uint64_t entryCount() const;
  uint64_t folderCount() const;

private:
  void validateOpened() const;

  void loadRootPages();

  void commit();

  void persistPreamble();

  void loadOrInitHeaderAndPreamble();
  void persistHeader();
  void processHeader(RawBytes &out) const;
  void unprocessHeader(const RawBytes &in);

  void initNewVault();
  bool verifyPassword();

  std::string path;
  std::string password;

  bool opened = false;

  vault::crypto::Key masterKey;

  StorageEngine storage;

  std::unique_ptr<Pager> pager;

  std::unique_ptr<DataManager> dataManager;
  std::unique_ptr<IndexManager> indexManager;

  std::unique_ptr<Codec> codec;
  std::shared_ptr<ICompressor> compressor;

  VaultPreamble preamble;
  VaultHeader header;
};
