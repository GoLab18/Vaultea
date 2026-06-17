#pragma once

#include "crypto/Constants.h"
#include "managers/DataManager.h"
#include "managers/IndexManager.h"
#include "models/Folder.h"
#include "models/VaultEntry.h"
#include "pipeline/Codec.h"
#include "storage/StorageEngine.h"
#include "storage/VaultHeader.h"
#include "storage/VaultPreamble.h"
#include "storage/page/Pager.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

/**
 * @class VaultEngine
 * @brief The primary facade and orchestrator for the Vaultea storage system.
 * * VaultEngine manages the lifecycle of a secure database. It acts as the
 * boundary between the frontend UI and the low-level storage, cryptography, and
 * managers.
 */
class VaultEngine {
public:
  /**
   * @brief Creates a new encrypted vault file on disk.
   * @param path The absolute or relative file path (e.g., "myvault.vtea").
   * @param password The plaintext master password used to derive the encryption
   * key.
   * @return True if creation succeeded, false if the file could not be created.
   */
  bool createVault(const std::string &path, const std::string &password);

  /**
   * @brief Opens and authenticates an existing vault file.
   * @param path The path to the vault file.
   * @param password The plaintext master password.
   * @return True if successful. False if the file is missing or the password is
   * wrong.
   */
  bool openVault(const std::string &path, const std::string &password);

  /**
   * @brief Flushes all pending writes to disk and closes the file handle.
   */
  void closeVault();

  /**
   * @brief Checks if a vault is currently loaded in memory.
   */
  bool isOpened() const;

  /**
   * @brief Encrypts and inserts a new entry into the vault.
   * @param entry The VaultEntry object containing credentials/notes.
   * @return The UUID string of the newly created entry.
   */
  std::string addEntry(const VaultEntry &entry);

  /**
   * @brief Updates an existing entry, potentially relocating it on disk.
   * @param entry The updated VaultEntry object. Must retain its original UUID.
   * @return True if the entry was found and updated.
   */
  bool updateEntry(const VaultEntry &entry);

  /**
   * @brief Permanently deletes an entry from both the index and disk.
   */
  bool deleteEntry(const std::string &id);

  /**
   * @brief Fetches and decrypts a specific entry by its UUID.
   * @return std::nullopt if the entry does not exist.
   */
  std::optional<VaultEntry> getEntry(const std::string &id);

  /**
   * @brief Decrypts and returns all entries in the vault.
   */
  std::vector<VaultEntry> getAllEntries();

  /**
   * @brief Returns all entries residing within a specific folder UUID.
   */
  std::vector<VaultEntry> getByFolder(const std::string &folderId);

  /**
   * @brief Performs a B-Tree prefix search on entry names.
   * @param query The string to search for (case-insensitive).
   */
  std::vector<VaultEntry> searchEntries(const std::string &query);

  /**
   * @brief Creates a new organizational folder.
   * @return The UUID string of the newly created folder.
   */
  std::string createFolder(const std::string &name);

  /**
   * @brief Updates folder metadata (name, description).
   */
  bool updateFolder(const Folder &folder);

  /**
   * @brief Deletes a folder.
   * @return False if the folder does not exist or if it still contains entries.
   */
  bool deleteFolder(const std::string &id);

  /**
   * @brief Fetches a specific folder by its UUID.
   */
  std::optional<Folder> getFolder(const std::string &id);

  /**
   * @brief Returns all folders in the vault.
   */
  std::vector<Folder> getFolders();

  /**
   * @brief Performs a B-Tree prefix search on folder names.
   */
  std::vector<Folder> searchFolders(const std::string &query);

  /**
   * @brief Reassigns an entry to a new folder.
   */
  bool addToFolder(const std::string &entryId, const std::string &folderId);

  /**
   * @brief Returns vault-level tracked entry count.
   */
  uint64_t entryCount() const;

  /**
   * @brief Returns vault-level tracked folder count.
   */
  uint64_t folderCount() const;

private:
  void validateOpened() const;

  void loadRootPages();
  void commit();
  void persistPreamble();
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

  VaultPreamble preamble;
  VaultHeader header;
};
