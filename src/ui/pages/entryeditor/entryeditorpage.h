#pragma once

#include "models/VaultEntry.h"

#include <QWidget>

namespace Ui {
class EntryEditorPage;
}
class VaultController;

class EntryEditorPage : public QWidget {
  Q_OBJECT
public:
  explicit EntryEditorPage(VaultController *controller,
                           QWidget *parent = nullptr);
  ~EntryEditorPage();

  void loadEntry(const QString &entryId);
  void prepareNewEntry();

signals:
  void backRequested();
  void saved();

private slots:
  void onTypeChanged(int index);
  void onSaveClicked();
  void onDeleteClicked();

private:
  Ui::EntryEditorPage *ui;
  VaultController *m_controller;
  QString m_currentEntryId;
  bool m_isNewEntry;

  void populateFolderCombo();
};
