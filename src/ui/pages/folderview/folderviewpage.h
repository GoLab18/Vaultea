#pragma once

#include <QString>
#include <QWidget>

namespace Ui {
class FolderViewPage;
}
class VaultController;
class QListWidgetItem;

class FolderViewPage : public QWidget {
  Q_OBJECT
public:
  explicit FolderViewPage(VaultController *controller,
                          QWidget *parent = nullptr);
  ~FolderViewPage();

  void loadFolder(const QString &folderIdStr, const QString &folderName);
  void refresh();

signals:
  void backRequested();
  void editEntryRequested(QString entryId);

private slots:
  void onDeleteFolderClicked();
  void onRenameFolderClicked();
  void onEntryDoubleClicked(QListWidgetItem *item);

private:
  Ui::FolderViewPage *ui;
  VaultController *m_controller;
  QString m_currentFolderId;
  QString m_currentFolderName;
};
