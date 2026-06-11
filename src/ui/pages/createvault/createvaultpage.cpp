#include "createvaultpage.h"
#include "../../widgets/passwordlineedit.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <qstyle.h>

CreateVaultPage::CreateVaultPage(QWidget *parent) : QWidget(parent) {
  auto *layout = new QVBoxLayout(this);
  layout->setAlignment(Qt::AlignCenter);

  auto *formWidget = new QWidget();
  formWidget->setMaximumWidth(500);
  auto *formLayout = new QVBoxLayout(formWidget);

  auto *title = new QLabel("<h2>Create New Vault</h2>");
  title->setAlignment(Qt::AlignCenter);

  auto *dirLayout = new QHBoxLayout();
  m_dirPath = new QLineEdit();
  m_dirPath->setReadOnly(true);
  m_dirPath->setPlaceholderText("Select directory...");
  auto *browseBtn =
      new QPushButton(style()->standardIcon(QStyle::SP_DirOpenIcon), "Browse");
  dirLayout->addWidget(m_dirPath);
  dirLayout->addWidget(browseBtn);

  m_fileName = new QLineEdit();
  m_fileName->setPlaceholderText("vault_name (will append .vtea)");

  m_password = new PasswordLineEdit();
  m_password->setPlaceholderText("Master Password");

  m_confirm = new PasswordLineEdit();
  m_confirm->setPlaceholderText("Confirm Password");

  auto *btnLayout = new QHBoxLayout();
  auto *backBtn = new QPushButton("Back");
  auto *createBtn = new QPushButton("Create");
  btnLayout->addWidget(backBtn);
  btnLayout->addWidget(createBtn);

  formLayout->addWidget(title);
  formLayout->addWidget(new QLabel("Vault Location:"));
  formLayout->addLayout(dirLayout);
  formLayout->addWidget(new QLabel("Vault Filename:"));
  formLayout->addWidget(m_fileName);
  formLayout->addWidget(new QLabel("Password:"));
  formLayout->addWidget(m_password);
  formLayout->addWidget(m_confirm);
  formLayout->addSpacing(20);
  formLayout->addLayout(btnLayout);

  layout->addWidget(formWidget);

  connect(browseBtn, &QPushButton::clicked, this, [this]() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory",
                                                    QDir::homePath());
    if (!dir.isEmpty())
      m_dirPath->setText(dir);
  });

  connect(backBtn, &QPushButton::clicked, this,
          &CreateVaultPage::backRequested);

  connect(createBtn, &QPushButton::clicked, this, [this]() {
    if (m_dirPath->text().isEmpty() || m_fileName->text().isEmpty()) {
      QMessageBox::warning(this, "Error",
                           "Please select a directory and filename.");
      return;
    }

    if (m_password->text().isEmpty() ||
        m_password->text() != m_confirm->text()) {
      QMessageBox::warning(this, "Error",
                           "Passwords do not match or are empty.");
      return;
    }

    QString fullPath = m_dirPath->text() + "/" + m_fileName->text();
    if (!fullPath.endsWith(".vtea"))
      fullPath += ".vtea";

    if (QFileInfo::exists(fullPath)) {
      QMessageBox::warning(this, "Error",
                           "A file already exists at this location. Please "
                           "choose a different name.");
      return;
    }

    emit createRequested(fullPath, m_password->text());
  });
}

void CreateVaultPage::clearFields() {
  m_dirPath->clear();
  m_fileName->clear();
  m_password->clear();
  m_confirm->clear();
}
