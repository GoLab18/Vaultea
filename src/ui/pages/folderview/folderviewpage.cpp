#include "folderviewpage.h"
#include "../../controllers/vaultcontroller.h"
#include "ui_folderviewpage.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QStyle>

FolderViewPage::FolderViewPage(VaultController *controller, QWidget *parent)
    : QWidget(parent), ui(new Ui::FolderViewPage), m_controller(controller) {
  ui->setupUi(this);

  ui->backButton->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
  ui->deleteFolderButton->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
  ui->renameFolderButton->setIcon(
      style()->standardIcon(QStyle::SP_BrowserReload));

  connect(ui->backButton, &QPushButton::clicked, this,
          &FolderViewPage::backRequested);
  connect(ui->deleteFolderButton, &QPushButton::clicked, this,
          &FolderViewPage::onDeleteFolderClicked);
  connect(ui->renameFolderButton, &QPushButton::clicked, this,
          &FolderViewPage::onRenameFolderClicked);
  connect(ui->entriesList, &QListWidget::itemDoubleClicked, this,
          &FolderViewPage::onEntryDoubleClicked);
}

FolderViewPage::~FolderViewPage() { delete ui; }

void FolderViewPage::loadFolder(const QString &folderIdStr,
                                const QString &folderName) {
  m_currentFolderId = folderIdStr;
  m_currentFolderName = folderName;
  ui->folderNameLabel->setText(m_currentFolderName);

  ui->entriesList->clear();
  auto entries = m_controller->getEntriesByFolder(folderIdStr);
  for (const auto &entry : entries) {
    auto *item = new QListWidgetItem(QString::fromStdString(entry.name));
    item->setData(Qt::UserRole, QString::fromStdString(entry.id.toString()));
    item->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    ui->entriesList->addItem(item);
  }
  ui->entryCountLabel->setText(
      QString("Total Entries: %1").arg(entries.size()));
}

void FolderViewPage::refresh() {
  loadFolder(m_currentFolderId, m_currentFolderName);
}

void FolderViewPage::onRenameFolderClicked() {
  bool ok;
  QString newName = QInputDialog::getText(this, "Rename Folder",
                                          "New Name:", QLineEdit::Normal,
                                          m_currentFolderName, &ok);
  if (ok && !newName.isEmpty()) {
    if (m_controller->renameFolder(m_currentFolderId, newName)) {
      m_currentFolderName = newName;
      ui->folderNameLabel->setText(m_currentFolderName);
    }
  }
}

void FolderViewPage::onDeleteFolderClicked() {
  if (m_controller->deleteFolderSafe(m_currentFolderId)) {
    emit backRequested();
  } else {
    QMessageBox::warning(
        this, "Cannot Delete",
        "This folder is not empty. Please delete or move all entries first.");
  }
}

void FolderViewPage::onEntryDoubleClicked(QListWidgetItem *item) {
  QString entryId = item->data(Qt::UserRole).toString();
  emit editEntryRequested(entryId);
}
