#include "vaultcontroller.h"

VaultController::VaultController(QObject *parent)
    : QObject(parent), m_engine(std::make_unique<VaultEngine>()) {}

VaultController::~VaultController() { closeVault(); }

bool VaultController::createVault(const QString &path,
                                  const QString &password) {
  return m_engine->createVault(path.toStdString(), password.toStdString());
}

bool VaultController::openVault(const QString &path, const QString &password) {
  return m_engine->openVault(path.toStdString(), password.toStdString());
}

void VaultController::closeVault() { m_engine->closeVault(); }

bool VaultController::isVaultOpen() const { return m_engine->isOpened(); }

QVector<VaultEntry> VaultController::searchEntries(const QString &query) {
  std::vector<VaultEntry> entries;
  if (query.isEmpty()) {
    entries = m_engine->getAllEntries();
  } else {
    entries = m_engine->searchEntries(query.toStdString());
  }

  return QVector<VaultEntry>(entries.begin(), entries.end());
}

QVector<Folder> VaultController::searchFolders(const QString &query) {
  std::vector<Folder> folders;
  if (query.isEmpty()) {
    folders = m_engine->getFolders();
  } else {
    folders = m_engine->searchFolders(query.toStdString());
  }

  return QVector<Folder>(folders.begin(), folders.end());
}

bool VaultController::deleteFolderSafe(const QString &folderIdStr) {
  auto entries = m_engine->getByFolder(folderIdStr.toStdString());
  if (!entries.empty()) {
    return false;
  }
  return m_engine->deleteFolder(folderIdStr.toStdString());
}

QString VaultController::addEntry(const VaultEntry &entry) {
  return QString::fromStdString(m_engine->addEntry(entry));
}

bool VaultController::updateEntry(const VaultEntry &entry) {
  return m_engine->updateEntry(entry);
}

bool VaultController::deleteEntry(const QString &entryId) {
  return m_engine->deleteEntry(entryId.toStdString());
}

std::optional<VaultEntry> VaultController::getEntry(const QString &entryId) {
  return m_engine->getEntry(entryId.toStdString());
}

QVector<VaultEntry>
VaultController::getEntriesByFolder(const QString &folderId) {
  auto entries = m_engine->getByFolder(folderId.toStdString());
  return QVector<VaultEntry>(entries.begin(), entries.end());
}

QString VaultController::createFolder(const QString &name) {
  return QString::fromStdString(m_engine->createFolder(name.toStdString()));
}

bool VaultController::renameFolder(const QString &id, const QString &newName) {
  return m_engine->renameFolder(id.toStdString(), newName.toStdString());
}
