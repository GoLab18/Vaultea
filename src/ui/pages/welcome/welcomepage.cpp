#include "welcomepage.h"
#include "../../theme/thememanager.h"
#include "ui_welcomepage.h"

WelcomePage::WelcomePage(QWidget *parent)
    : QWidget(parent), ui(new Ui::WelcomePage) {

  ui->setupUi(this);

  QIcon lockIcon = ThemeManager::colorizedIcon(
      ":/assets/icons/lock.svg", ThemeManager::getAccentColor(), 64);
  ui->iconLabel->setPixmap(lockIcon.pixmap(64, 64));

  QColor iconColor = ThemeManager::getTextColor();
  ui->createBtn->setIcon(
      ThemeManager::colorizedIcon(":/assets/icons/vault.svg", iconColor));
  ui->openBtn->setIcon(
      ThemeManager::colorizedIcon(":/assets/icons/folder.svg", iconColor));

  connect(ui->createBtn, &QPushButton::clicked, this,
          &WelcomePage::createRequested);
  connect(ui->openBtn, &QPushButton::clicked, this,
          &WelcomePage::openRequested);
}

WelcomePage::~WelcomePage() { delete ui; }
