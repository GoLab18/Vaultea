#include "openvaultpage.h"
#include "../../widgets/passwordlineedit.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <qlineedit.h>
#include <qstyle.h>

OpenVaultPage::OpenVaultPage(QWidget *parent) : QWidget(parent) {
  auto *layout = new QVBoxLayout(this);
  layout->setAlignment(Qt::AlignCenter);

  auto *formWidget = new QWidget();
  formWidget->setMaximumWidth(500);
  auto *formLayout = new QVBoxLayout(formWidget);

  auto *title = new QLabel("<h2>Open Vault</h2>");
  title->setAlignment(Qt::AlignCenter);

  auto *fileLayout = new QHBoxLayout();
  m_path = new QLineEdit();
  m_path->setReadOnly(true);
  m_path->setPlaceholderText("Select .vtea file...");
  auto *browseBtn = new QPushButton(
      style()->standardIcon(QStyle::SP_DialogOpenButton), "Browse");
  fileLayout->addWidget(m_path);
  fileLayout->addWidget(browseBtn);

  m_password = new PasswordLineEdit();
  m_password->setPlaceholderText("Master Password");

  auto *btnLayout = new QHBoxLayout();
  auto *backBtn = new QPushButton("Back");
  auto *openBtn = new QPushButton("Open");
  btnLayout->addWidget(backBtn);
  btnLayout->addWidget(openBtn);

  formLayout->addWidget(title);
  formLayout->addWidget(new QLabel("Vault File:"));
  formLayout->addLayout(fileLayout);
  formLayout->addWidget(new QLabel("Password:"));
  formLayout->addWidget(m_password);
  formLayout->addSpacing(20);
  formLayout->addLayout(btnLayout);

  layout->addWidget(formWidget);

  connect(browseBtn, &QPushButton::clicked, this, [this]() {
    QString file =
        QFileDialog::getOpenFileName(this, "Select Vault", QDir::homePath(),
                                     "Vault Files (*.vtea);;All Files (*)");
    if (!file.isEmpty())
      m_path->setText(file);
  });

  connect(backBtn, &QPushButton::clicked, this, &OpenVaultPage::backRequested);
  connect(openBtn, &QPushButton::clicked, this,
          [this]() { emit openRequested(m_path->text(), m_password->text()); });
}
