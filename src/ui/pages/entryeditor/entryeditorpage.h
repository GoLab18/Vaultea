#pragma once

#include "models/VaultEntry.h"

#include <QVector>
#include <QWidget>
#include <cstdint>

class QLineEdit;
class QHBoxLayout;

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
  void addUrlRow(const QString &url = "");
  void copyToClipboard(const QString &text);

private:
  Ui::EntryEditorPage *ui;
  VaultController *m_controller;
  QString m_currentEntryId;
  bool m_isNewEntry;

  uint64_t m_currentCreatedAt = 0;

  QVector<QLineEdit *> m_urlFields;

  void populateFolderCombo();
  void setupValidators();
  void clearDynamicUrls();
  void setTimestamps(uint64_t created, uint64_t updated);
  void updateStackedWidgetSize();
};
