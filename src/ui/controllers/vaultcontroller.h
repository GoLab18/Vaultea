#pragma once

#include "core/VaultEngine.h"

#include <QObject>
#include <QString>
#include <QVector>
#include <memory>
#include <optional>

class VaultController : public QObject {
  Q_OBJECT
public:
  explicit VaultController(QObject *parent = nullptr);
  ~VaultController();

  bool createVault(const QString &path, const QString &password);
  bool openVault(const QString &path, const QString &password);
  void closeVault();
  bool isVaultOpen() const;

  QVector<VaultEntry> searchEntries(const QString &query);
  QVector<Folder> searchFolders(const QString &query);

  QString addEntry(const VaultEntry &entry);
  bool updateEntry(const VaultEntry &entry);
  bool deleteEntry(const QString &entryId);
  std::optional<VaultEntry> getEntry(const QString &entryId);
  QVector<VaultEntry> getEntriesByFolder(const QString &folderId);

  QString createFolder(const QString &name);
  bool updateFolder(const Folder &folder);
  bool deleteFolderSafe(const QString &folderIdStr);
  std::optional<Folder> getFolder(const QString &folderId);

private:
  std::unique_ptr<VaultEngine> m_engine;
};
