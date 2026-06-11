#include "entryeditorpage.h"
#include "../../../util/Helpers.h"
#include "../../controllers/vaultcontroller.h"
#include "../../theme/thememanager.h"
#include "../../widgets/passwordlineedit.h"
#include "ui_entryeditorpage.h"

#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QDateTime>
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include <QTimer>
#include <QToolButton>
#include <QToolTip>

EntryEditorPage::EntryEditorPage(VaultController *controller, QWidget *parent)
    : QWidget(parent), ui(new Ui::EntryEditorPage), m_controller(controller) {
  ui->setupUi(this);

  setupValidators();

  QColor iconColor = ThemeManager::getTextColor();
  QIcon copyIcon =
      ThemeManager::colorizedIcon(":/assets/icons/copy.svg", iconColor, 18);
  ui->copyUserBtn->setIcon(copyIcon);
  ui->copyPassBtn->setIcon(copyIcon);
  ui->copyCardBtn->setIcon(copyIcon);

  connect(ui->cancelButton, &QPushButton::clicked, this,
          &EntryEditorPage::backRequested);
  connect(ui->saveButton, &QPushButton::clicked, this,
          &EntryEditorPage::onSaveClicked);
  connect(ui->deleteButton, &QPushButton::clicked, this,
          &EntryEditorPage::onDeleteClicked);
  connect(ui->typeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &EntryEditorPage::onTypeChanged);
  connect(ui->addUrlBtn, &QPushButton::clicked, this,
          [this]() { addUrlRow(); });

  connect(ui->copyUserBtn, &QToolButton::clicked, this,
          [this]() { copyToClipboard(ui->usernameEdit->text()); });
  connect(ui->copyPassBtn, &QToolButton::clicked, this,
          [this]() { copyToClipboard(ui->passwordEdit->text()); });
  connect(ui->copyCardBtn, &QToolButton::clicked, this,
          [this]() { copyToClipboard(ui->cardNumberEdit->text()); });
}

EntryEditorPage::~EntryEditorPage() { delete ui; }

void EntryEditorPage::setupValidators() {
  QRegularExpression cardRegex("^[0-9]{13,19}$");
  ui->cardNumberEdit->setValidator(
      new QRegularExpressionValidator(cardRegex, this));

  QRegularExpression cvvRegex("^[0-9]{3,4}$");
  ui->cardCvvEdit->setValidator(
      new QRegularExpressionValidator(cvvRegex, this));

  QRegularExpression expRegex("^(0[1-9]|1[0-2])\\/([0-9]{2})$");
  ui->cardExpiryEdit->setValidator(
      new QRegularExpressionValidator(expRegex, this));
}

void EntryEditorPage::copyToClipboard(const QString &text) {
  if (text.isEmpty())
    return;
  QApplication::clipboard()->setText(text);

  QToolTip::showText(QCursor::pos(), "Copied to clipboard!", this, QRect(),
                     1500);
}

void EntryEditorPage::addUrlRow(const QString &url) {
  auto *rowWidget = new QWidget(ui->urlsContainer);
  auto *layout = new QHBoxLayout(rowWidget);
  layout->setContentsMargins(0, 0, 0, 0);

  auto *lineEdit = new QLineEdit(url, rowWidget);
  auto *removeBtn = new QToolButton(rowWidget);

  removeBtn->setIcon(ThemeManager::colorizedIcon(
      ":/assets/icons/trash.svg", ThemeManager::getAccentColor(), 18));

  layout->addWidget(lineEdit);
  layout->addWidget(removeBtn);

  ui->urlsLayout->addWidget(rowWidget);
  m_urlFields.append(lineEdit);

  updateStackedWidgetSize();

  connect(
      removeBtn, &QToolButton::clicked, this, [this, rowWidget, lineEdit]() {
        m_urlFields.removeOne(lineEdit);
        rowWidget->deleteLater();

        QTimer::singleShot(0, this, &EntryEditorPage::updateStackedWidgetSize);
      });
}

void EntryEditorPage::clearDynamicUrls() {
  for (auto *field : m_urlFields) {
    field->parentWidget()->deleteLater();
  }
  m_urlFields.clear();
}

void EntryEditorPage::setTimestamps(uint64_t created, uint64_t updated) {
  if (created == 0) {
    ui->timestampsLabel->setText("New Entry");
    return;
  }

  QString formatStr = "yyyy-MM-dd HH:mm";

  QString cStr = QDateTime::fromSecsSinceEpoch(created).toString(formatStr);
  QString uStr = QDateTime::fromSecsSinceEpoch(updated).toString(formatStr);

  ui->timestampsLabel->setText(
      QString("Created: %1 | Updated: %2").arg(cStr, uStr));
}

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
  m_currentCreatedAt = vault::util::time::now();

  ui->nameLineEdit->clear();
  ui->notesEdit->clear();
  ui->deleteButton->hide();

  ui->usernameEdit->clear();
  ui->passwordEdit->clear();

  ui->cardTypeCombo->setCurrentIndex(0);
  ui->cardHolderEdit->clear();
  ui->cardNumberEdit->clear();
  ui->cardExpiryEdit->clear();
  ui->cardCvvEdit->clear();

  ui->sexCombo->setCurrentIndex(0);
  ui->firstNameEdit->clear();
  ui->middleNameEdit->clear();
  ui->lastNameEdit->clear();
  ui->idUsernameEdit->clear();
  ui->companyEdit->clear();
  ui->ssnEdit->clear();
  ui->passportEdit->clear();
  ui->licenseEdit->clear();
  ui->emailEdit->clear();
  ui->phoneEdit->clear();
  ui->cityEdit->clear();
  ui->stateEdit->clear();
  ui->zipEdit->clear();
  ui->countryEdit->clear();

  ui->secureNoteContent->clear();

  populateFolderCombo();
  ui->typeComboBox->setCurrentIndex(0);
  clearDynamicUrls();
  addUrlRow();

  setTimestamps(m_currentCreatedAt, m_currentCreatedAt);

  updateStackedWidgetSize();
}

void EntryEditorPage::loadEntry(const QString &entryId) {
  m_isNewEntry = false;
  m_currentEntryId = entryId;
  ui->deleteButton->show();

  populateFolderCombo();
  clearDynamicUrls();

  auto optEntry = m_controller->getEntry(entryId);
  if (!optEntry.has_value())
    return;

  VaultEntry entry = optEntry.value();

  m_currentCreatedAt = entry.createdAt;

  ui->nameLineEdit->setText(QString::fromStdString(entry.name));
  ui->notesEdit->setPlainText(QString::fromStdString(entry.notes));
  setTimestamps(entry.createdAt, entry.updatedAt);

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

    for (const auto &u : d.urls)
      addUrlRow(QString::fromStdString(u));

    if (m_urlFields.isEmpty())
      addUrlRow();
    break;
  }

  case EntryType::Card: {
    auto d = std::get<CardData>(entry.data);

    ui->cardTypeCombo->setCurrentIndex(static_cast<int>(d.type));

    ui->cardHolderEdit->setText(QString::fromStdString(d.holder));
    ui->cardNumberEdit->setText(QString::fromStdString(d.number));
    ui->cardExpiryEdit->setText(QString::fromStdString(d.expiry));
    ui->cardCvvEdit->setText(QString::fromStdString(d.cvv));

    break;
  }

  case EntryType::Identity: {
    auto d = std::get<IdentityData>(entry.data);

    ui->sexCombo->setCurrentIndex(static_cast<int>(d.sex));

    ui->firstNameEdit->setText(QString::fromStdString(d.firstName));
    ui->middleNameEdit->setText(QString::fromStdString(d.middleName));
    ui->lastNameEdit->setText(QString::fromStdString(d.lastName));
    ui->idUsernameEdit->setText(QString::fromStdString(d.username));
    ui->companyEdit->setText(QString::fromStdString(d.company));

    ui->ssnEdit->setText(QString::fromStdString(d.ssn));
    ui->passportEdit->setText(QString::fromStdString(d.passportNumber));
    ui->licenseEdit->setText(QString::fromStdString(d.licenseNumber));

    ui->emailEdit->setText(QString::fromStdString(d.email));
    ui->phoneEdit->setText(QString::fromStdString(d.phone));

    ui->cityEdit->setText(QString::fromStdString(d.city));
    ui->stateEdit->setText(QString::fromStdString(d.state));
    ui->zipEdit->setText(QString::fromStdString(d.zip));
    ui->countryEdit->setText(QString::fromStdString(d.country));

    break;
  }

  case EntryType::Note: {
    auto d = std::get<SecureNoteData>(entry.data);

    ui->secureNoteContent->setPlainText(QString::fromStdString(d.content));

    break;
  }
  }

  updateStackedWidgetSize();
}

void EntryEditorPage::onTypeChanged(int index) {
  ui->typeStackedWidget->setCurrentIndex(index);
  updateStackedWidgetSize();
}

void EntryEditorPage::onSaveClicked() {
  VaultEntry entry;

  entry.createdAt = m_currentCreatedAt;
  entry.updatedAt = vault::util::time::now();

  entry.name = ui->nameLineEdit->text().toStdString();
  entry.notes = ui->notesEdit->toPlainText().toStdString();

  QString folderIdStr = ui->folderComboBox->currentData().toString();
  if (!folderIdStr.isEmpty())
    entry.folderId = UUID::fromString(folderIdStr.toStdString());

  entry.type = static_cast<EntryType>(ui->typeComboBox->currentIndex());

  switch (entry.type) {

  case EntryType::Login: {
    std::vector<std::string> parsedUrls;
    for (auto *field : m_urlFields) {
      if (!field->text().trimmed().isEmpty())
        parsedUrls.push_back(field->text().trimmed().toStdString());
    }

    entry.data = LoginData{ui->usernameEdit->text().toStdString(),
                           ui->passwordEdit->text().toStdString(), parsedUrls};

    break;
  }

  case EntryType::Card:
    entry.data =
        CardData{static_cast<CardType>(ui->cardTypeCombo->currentIndex()),
                 ui->cardHolderEdit->text().toStdString(),
                 ui->cardNumberEdit->text().toStdString(),
                 ui->cardExpiryEdit->text().toStdString(),
                 ui->cardCvvEdit->text().toStdString()};

    break;

  case EntryType::Identity:
    entry.data = IdentityData{static_cast<Sex>(ui->sexCombo->currentIndex()),
                              ui->firstNameEdit->text().toStdString(),
                              ui->middleNameEdit->text().toStdString(),
                              ui->lastNameEdit->text().toStdString(),
                              ui->idUsernameEdit->text().toStdString(),
                              ui->companyEdit->text().toStdString(),
                              ui->ssnEdit->text().toStdString(),
                              ui->passportEdit->text().toStdString(),
                              ui->licenseEdit->text().toStdString(),
                              ui->emailEdit->text().toStdString(),
                              ui->phoneEdit->text().toStdString(),
                              ui->cityEdit->text().toStdString(),
                              ui->stateEdit->text().toStdString(),
                              ui->zipEdit->text().toStdString(),
                              ui->countryEdit->text().toStdString()};
    break;

  case EntryType::Note:
    entry.data =
        SecureNoteData{ui->secureNoteContent->toPlainText().toStdString()};

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

void EntryEditorPage::updateStackedWidgetSize() {
  for (int i = 0; i < ui->typeStackedWidget->count(); ++i) {
    QWidget *page = ui->typeStackedWidget->widget(i);
    if (i == ui->typeStackedWidget->currentIndex()) {
      page->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    } else {
      page->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    }
  }

  ui->typeStackedWidget->updateGeometry();
  ui->typeStackedWidget->adjustSize();
  this->adjustSize();
}
