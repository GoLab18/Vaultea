#include "vaultpage.h"
#include "../../controllers/vaultcontroller.h"
#include "../../theme/thememanager.h"
#include "ui_vaultpage.h"

#include <QInputDialog>
#include <QStyle>

VaultPage::VaultPage(VaultController *controller, QWidget *parent)
    : QWidget(parent), ui(new Ui::VaultPage), m_controller(controller) {
  ui->setupUi(this);

  ui->searchBar->addAction(
      style()->standardIcon(QStyle::SP_MessageBoxInformation),
      QLineEdit::LeadingPosition);
  ui->lockBtn->setIcon(style()->standardIcon(QStyle::SP_BrowserStop));
  ui->newEntryBtn->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
  ui->newFolderBtn->setIcon(style()->standardIcon(QStyle::SP_DirIcon));

  connect(ui->lockBtn, &QPushButton::clicked, this, &VaultPage::lockRequested);
  connect(ui->searchBar, &QLineEdit::textChanged, this,
          &VaultPage::performSearch);
  connect(ui->foldersList, &QListWidget::itemDoubleClicked, this,
          &VaultPage::onFolderClicked);
  connect(ui->entriesList, &QListWidget::itemDoubleClicked, this,
          &VaultPage::onEntryClicked);
  connect(ui->newEntryBtn, &QPushButton::clicked, this,
          [this]() { emit newEntryRequested(); });
  connect(ui->newFolderBtn, &QPushButton::clicked, this,
          &VaultPage::onNewFolderClicked);
}

VaultPage::~VaultPage() { delete ui; }

void VaultPage::refreshView() { performSearch(ui->searchBar->text()); }

void VaultPage::performSearch(const QString &query) {
  ui->entriesList->clear();

  auto entries = m_controller->searchEntries(query);
  for (const auto &entry : entries) {
    auto *item = new QListWidgetItem(QString::fromStdString(entry.name));
    item->setData(Qt::UserRole, QString::fromStdString(entry.id.toString()));

    item->setIcon(ThemeManager::iconForEntryType(entry.type));

    ui->entriesList->addItem(item);
  }

  ui->foldersList->clear();
  auto folders = m_controller->searchFolders(query);
  for (const auto &folder : folders) {
    auto entriesInFolder = m_controller->getEntriesByFolder(
        QString::fromStdString(folder.id.toString()));

    QString title = QString::fromStdString(folder.name) + "      (" +
                    QString::number(entriesInFolder.size()) + ")";

    auto *item = new QListWidgetItem(title);
    item->setData(Qt::UserRole, QString::fromStdString(folder.id.toString()));

    item->setIcon(ThemeManager::colorizedIcon(":/assets/icons/folder.svg",
                                              ThemeManager::getTextColor()));

    ui->foldersList->addItem(item);
  }
}

void VaultPage::onFolderClicked(QListWidgetItem *item) {
  QString id = item->data(Qt::UserRole).toString();
  QString name = item->text();
  emit folderRequested(id, name);
}

void VaultPage::onEntryClicked(QListWidgetItem *item) {
  QString id = item->data(Qt::UserRole).toString();
  emit editEntryRequested(id);
}

void VaultPage::onNewFolderClicked() {
  bool ok;
  QString name = QInputDialog::getText(
      this, "New Folder", "Folder Name:", QLineEdit::Normal, "", &ok);
  if (ok && !name.isEmpty()) {
    m_controller->createFolder(name);
    refreshView();
  }
}
