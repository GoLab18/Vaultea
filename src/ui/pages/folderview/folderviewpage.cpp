#include "folderviewpage.h"
#include "../../controllers/vaultcontroller.h"
#include "../../theme/thememanager.h"
#include "ui/pages/folderview/editfolderdialog.h"
#include "ui_folderviewpage.h"

#include <QInputDialog>
#include <QMessageBox>

FolderViewPage::FolderViewPage(VaultController *controller, QWidget *parent)
    : QWidget(parent), ui(new Ui::FolderViewPage), m_controller(controller) {
  ui->setupUi(this);

  QColor iconColor = ThemeManager::getTextColor();

  ui->backButton->setIcon(
      ThemeManager::colorizedIcon(":/assets/icons/arrow-left.svg", iconColor));
  ui->deleteFolderButton->setIcon(
      ThemeManager::colorizedIcon(":/assets/icons/trash.svg", iconColor));
  ui->editFolderButton->setIcon(
      ThemeManager::colorizedIcon(":/assets/icons/pencil.svg", iconColor));

  connect(ui->backButton, &QPushButton::clicked, this,
          &FolderViewPage::backRequested);
  connect(ui->deleteFolderButton, &QPushButton::clicked, this,
          &FolderViewPage::onDeleteFolderClicked);
  connect(ui->editFolderButton, &QPushButton::clicked, this,
          &FolderViewPage::onEditFolderClicked);
  connect(ui->entriesList, &QListWidget::itemDoubleClicked, this,
          &FolderViewPage::onEntryDoubleClicked);
}

FolderViewPage::~FolderViewPage() { delete ui; }

void FolderViewPage::loadFolder(const QString &folderIdStr,
                                const QString &folderName) {
  m_currentFolderId = folderIdStr;

  auto optFolder = m_controller->getFolder(folderIdStr);
  if (optFolder.has_value()) {
    m_currentFolderName = QString::fromStdString(optFolder.value().name);
    m_currentFolderDesc = QString::fromStdString(optFolder.value().description);

    int countIndex = m_currentFolderName.lastIndexOf("      (");
    if (countIndex != -1) {
      m_currentFolderName = m_currentFolderName.left(countIndex);
    }

    ui->folderNameLabel->setText(m_currentFolderName);

    if (!m_currentFolderDesc.isEmpty()) {
      ui->folderDescLabel->setText(m_currentFolderDesc);
      ui->folderDescLabel->show();
    } else {
      ui->folderDescLabel->clear();
      ui->folderDescLabel->hide();
    }
  } else {
    m_currentFolderName = folderName;
    ui->folderNameLabel->setText(m_currentFolderName);
  }

  ui->entriesList->clear();
  auto entries = m_controller->getEntriesByFolder(folderIdStr);
  for (const auto &entry : entries) {
    auto *item = new QListWidgetItem(QString::fromStdString(entry.name));
    item->setData(Qt::UserRole, QString::fromStdString(entry.id.toString()));

    item->setIcon(ThemeManager::iconForEntryType(entry.type));

    ui->entriesList->addItem(item);
  }

  ui->entryCountLabel->setText(
      QString("Total Entries: %1").arg(entries.size()));
}

void FolderViewPage::refresh() {
  loadFolder(m_currentFolderId, m_currentFolderName);
}

void FolderViewPage::onEditFolderClicked() {
  EditFolderDialog dialog(this);
  dialog.setInitialData(m_currentFolderName, m_currentFolderDesc);

  if (dialog.exec() == QDialog::Accepted) {
    auto optFolder = m_controller->getFolder(m_currentFolderId);
    if (optFolder.has_value()) {
      Folder f = optFolder.value();
      f.name = dialog.name().toStdString();
      f.description = dialog.description().toStdString();

      if (m_controller->updateFolder(f)) {
        refresh();
      }
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
