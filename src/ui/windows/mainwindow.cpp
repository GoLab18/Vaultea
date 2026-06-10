#include "mainwindow.h"
#include "../controllers/vaultcontroller.h"
#include "../pages/createvault/createvaultpage.h"
#include "../pages/entryeditor/entryeditorpage.h"
#include "../pages/folderview/folderviewpage.h"
#include "../pages/openvault/openvaultpage.h"
#include "../pages/vault/vaultpage.h"
#include "../pages/welcome/welcomepage.h"
#include "../theme/thememanager.h"

#include <QApplication>
#include <QMenuBar>
#include <QMessageBox>

MainWindow::MainWindow() {
  m_controller = new VaultController(this);
  m_stack = new QStackedWidget(this);
  setCentralWidget(m_stack);

  setupMenuBar();

  m_welcome = new WelcomePage();
  m_create = new CreateVaultPage();
  m_open = new OpenVaultPage();
  m_vault = new VaultPage(m_controller);
  m_folderView = new FolderViewPage(m_controller);
  m_entryEditor = new EntryEditorPage(m_controller);

  m_stack->addWidget(m_welcome);
  m_stack->addWidget(m_create);
  m_stack->addWidget(m_open);
  m_stack->addWidget(m_vault);
  m_stack->addWidget(m_folderView);
  m_stack->addWidget(m_entryEditor);

  m_stack->setCurrentWidget(m_welcome);

  m_logoutTimer = new QTimer(this);
  m_logoutTimer->setInterval(5 * 60 * 1000);
  connect(m_logoutTimer, &QTimer::timeout, this, &MainWindow::handleLogout);
  qApp->installEventFilter(this);

  connect(m_welcome, &WelcomePage::createRequested,
          [this]() { m_stack->setCurrentWidget(m_create); });
  connect(m_welcome, &WelcomePage::openRequested,
          [this]() { m_stack->setCurrentWidget(m_open); });
  connect(m_create, &CreateVaultPage::backRequested,
          [this]() { m_stack->setCurrentWidget(m_welcome); });
  connect(m_open, &OpenVaultPage::backRequested,
          [this]() { m_stack->setCurrentWidget(m_welcome); });
  connect(m_vault, &VaultPage::lockRequested, this, &MainWindow::handleLogout);

  connect(m_create, &CreateVaultPage::createRequested,
          [this](QString path, QString password) {
            if (m_controller->createVault(path, password)) {
              m_vault->refreshView();
              m_stack->setCurrentWidget(m_vault);
              m_logoutTimer->start();
            } else {
              QMessageBox::warning(this, "Error", "Vault creation failed.");
            }
          });

  connect(m_open, &OpenVaultPage::openRequested,
          [this](QString path, QString password) {
            if (m_controller->openVault(path, password)) {
              m_vault->refreshView();
              m_stack->setCurrentWidget(m_vault);
              m_logoutTimer->start();
            } else {
              QMessageBox::warning(
                  this, "Error",
                  "Failed to open vault. Check password or file.");
            }
          });

  connect(m_vault, &VaultPage::folderRequested,
          [this](QString id, QString name) {
            m_folderView->loadFolder(id, name);
            m_stack->setCurrentWidget(m_folderView);
          });

  connect(m_vault, &VaultPage::newEntryRequested, [this]() {
    m_entryEditor->prepareNewEntry();
    m_previousPage = m_vault;
    m_stack->setCurrentWidget(m_entryEditor);
  });

  connect(m_vault, &VaultPage::editEntryRequested, [this](QString id) {
    m_entryEditor->loadEntry(id);
    m_previousPage = m_vault;
    m_stack->setCurrentWidget(m_entryEditor);
  });

  connect(m_folderView, &FolderViewPage::backRequested, [this]() {
    m_vault->refreshView();
    m_stack->setCurrentWidget(m_vault);
  });

  connect(m_folderView, &FolderViewPage::editEntryRequested,
          [this](QString id) {
            m_entryEditor->loadEntry(id);
            m_previousPage = m_folderView;
            m_stack->setCurrentWidget(m_entryEditor);
          });

  auto returnFromEditor = [this]() {
    if (m_previousPage == m_folderView) {
      m_folderView->refresh();
      m_stack->setCurrentWidget(m_folderView);
    } else {
      m_vault->refreshView();
      m_stack->setCurrentWidget(m_vault);
    }
  };

  connect(m_entryEditor, &EntryEditorPage::backRequested, returnFromEditor);
  connect(m_entryEditor, &EntryEditorPage::saved, returnFromEditor);
}

void MainWindow::setupMenuBar() {
  QMenu *settingsMenu = menuBar()->addMenu("Settings");
  QAction *themeAction = settingsMenu->addAction("Toggle Light/Dark Theme");
  connect(themeAction, &QAction::triggered, this, &MainWindow::toggleTheme);
}

void MainWindow::toggleTheme() {
  m_isDarkTheme = !m_isDarkTheme;
  ThemeManager::applyTheme(m_isDarkTheme ? AppTheme::Dark : AppTheme::Light);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
  if (event->type() == QEvent::KeyPress || event->type() == QEvent::MouseMove ||
      event->type() == QEvent::MouseButtonPress) {
    resetLogoutTimer();
  }
  return QMainWindow::eventFilter(obj, event);
}

void MainWindow::resetLogoutTimer() {
  if (m_logoutTimer->isActive())
    m_logoutTimer->start();
}

void MainWindow::handleLogout() {
  m_logoutTimer->stop();
  bool wasOpen = m_controller->isVaultOpen();
  m_controller->closeVault();
  m_stack->setCurrentWidget(m_welcome);

  if (wasOpen) {
    QMessageBox::information(this, "Vault Locked",
                             "Your session has expired or was manually locked. "
                             "Your data is secure.");
  }
}
