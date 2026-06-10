#include "entryeditorpage.h"
#include "../../controllers/vaultcontroller.h"
#include "ui_entryeditorpage.h"

#include <QMessageBox>
#include <QStringList>

EntryEditorPage::EntryEditorPage(VaultController *controller, QWidget *parent)
    : QWidget(parent), ui(new Ui::EntryEditorPage), m_controller(controller) {
  ui->setupUi(this);

  connect(ui->cancelButton, &QPushButton::clicked, this,
          &EntryEditorPage::backRequested);
  connect(ui->saveButton, &QPushButton::clicked, this,
          &EntryEditorPage::onSaveClicked);
  connect(ui->deleteButton, &QPushButton::clicked, this,
          &EntryEditorPage::onDeleteClicked);
  connect(ui->typeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &EntryEditorPage::onTypeChanged);
}

EntryEditorPage::~EntryEditorPage() { delete ui; }

void EntryEditorPage::populateFolderCombo() {
  ui->folderComboBox->clear();
  ui->folderComboBox->addItem("No Folder", "");
  auto folders = m_controller->searchFolders("");
  for (const auto &f : folders) {
    ui->folderComboBox->addItem(QString::fromStdString(f.name),
                                QString::fromStdString(f.id.toString()));
  }
}

void EntryEditorPage::prepareNewEntry() {
  m_isNewEntry = true;
  m_currentEntryId.clear();
  ui->nameLineEdit->clear();
  ui->deleteButton->hide();
  populateFolderCombo();
  ui->typeComboBox->setCurrentIndex(0);
}

void EntryEditorPage::loadEntry(const QString &entryId) {
  m_isNewEntry = false;
  m_currentEntryId = entryId;
  ui->deleteButton->show();
  populateFolderCombo();

  auto optEntry = m_controller->getEntry(entryId);
  if (!optEntry.has_value())
    return;

  VaultEntry entry = optEntry.value();
  ui->nameLineEdit->setText(QString::fromStdString(entry.name));

  int folderIdx = ui->folderComboBox->findData(
      QString::fromStdString(entry.folderId.toString()));
  if (folderIdx != -1)
    ui->folderComboBox->setCurrentIndex(folderIdx);

  int typeIdx = static_cast<int>(entry.type);
  ui->typeComboBox->setCurrentIndex(typeIdx);

  switch (entry.type) {
  case EntryType::Login: {
    auto d = std::get<LoginData>(entry.data);
    ui->usernameEdit->setText(QString::fromStdString(d.username));
    ui->passwordEdit->setText(QString::fromStdString(d.password));

    QStringList urlList;
    for (const auto &u : d.urls) {
      urlList << QString::fromStdString(u);
    }
    ui->urlsEdit->setText(urlList.join(", "));
    break;
  }
  case EntryType::Card: {
    auto d = std::get<CardData>(entry.data);
    ui->cardHolderEdit->setText(QString::fromStdString(d.holder));
    ui->cardNumberEdit->setText(QString::fromStdString(d.number));
    ui->cardExpiryEdit->setText(QString::fromStdString(d.expiry));
    ui->cardCvvEdit->setText(QString::fromStdString(d.cvv));
    break;
  }
  case EntryType::Identity: {
    auto d = std::get<IdentityData>(entry.data);
    ui->fullNameEdit->setText(QString::fromStdString(d.fullName));
    ui->emailEdit->setText(QString::fromStdString(d.email));
    ui->phoneEdit->setText(QString::fromStdString(d.phone));
    break;
  }
  case EntryType::Note: {
    auto d = std::get<SecureNoteData>(entry.data);
    ui->noteContentEdit->setPlainText(QString::fromStdString(d.content));
    break;
  }
  }
}

void EntryEditorPage::onTypeChanged(int index) {
  ui->typeStackedWidget->setCurrentIndex(index);
}

void EntryEditorPage::onSaveClicked() {
  VaultEntry entry;
  entry.name = ui->nameLineEdit->text().toStdString();

  QString folderIdStr = ui->folderComboBox->currentData().toString();
  if (!folderIdStr.isEmpty()) {
    entry.folderId = UUID::fromString(folderIdStr.toStdString());
  }

  entry.type = static_cast<EntryType>(ui->typeComboBox->currentIndex());

  switch (entry.type) {
  case EntryType::Login: {
    std::vector<std::string> parsedUrls;
    QStringList splitUrls = ui->urlsEdit->text().split(",", Qt::SkipEmptyParts);
    for (const auto &u : splitUrls) {
      parsedUrls.push_back(u.trimmed().toStdString());
    }

    entry.data = LoginData{ui->usernameEdit->text().toStdString(),
                           ui->passwordEdit->text().toStdString(), parsedUrls};
    break;
  }
  case EntryType::Card:
    entry.data = CardData{ui->cardHolderEdit->text().toStdString(),
                          ui->cardNumberEdit->text().toStdString(),
                          ui->cardExpiryEdit->text().toStdString(),
                          ui->cardCvvEdit->text().toStdString()};
    break;
  case EntryType::Identity:
    entry.data = IdentityData{ui->fullNameEdit->text().toStdString(),
                              ui->emailEdit->text().toStdString(),
                              ui->phoneEdit->text().toStdString()};
    break;
  case EntryType::Note:
    entry.data =
        SecureNoteData{ui->noteContentEdit->toPlainText().toStdString()};
    break;
  }

  if (m_isNewEntry) {
    entry.id = UUID::random();
    m_controller->addEntry(entry);
  } else {
    entry.id = UUID::fromString(m_currentEntryId.toStdString());
    m_controller->updateEntry(entry);
  }

  emit saved();
}

void EntryEditorPage::onDeleteClicked() {
  if (!m_isNewEntry) {
    m_controller->deleteEntry(m_currentEntryId);
    emit backRequested();
  }
}
