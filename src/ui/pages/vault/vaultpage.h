#pragma once

#include <QString>
#include <QWidget>

namespace Ui {
class VaultPage;
}
class VaultController;
class QListWidgetItem;

class VaultPage : public QWidget {
  Q_OBJECT
public:
  explicit VaultPage(VaultController *controller, QWidget *parent = nullptr);
  ~VaultPage();
  void refreshView();

signals:
  void lockRequested();
  void folderRequested(QString id, QString name);
  void newEntryRequested();
  void editEntryRequested(QString id);

private slots:
  void performSearch(const QString &query);
  void onFolderClicked(QListWidgetItem *item);
  void onEntryClicked(QListWidgetItem *item);
  void onNewFolderClicked();

private:
  Ui::VaultPage *ui;
  VaultController *m_controller;
};
