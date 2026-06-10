#pragma once

#include <QEvent>
#include <QMainWindow>
#include <QStackedWidget>
#include <QTimer>

class VaultController;
class WelcomePage;
class CreateVaultPage;
class OpenVaultPage;
class VaultPage;
class FolderViewPage;
class EntryEditorPage;

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  MainWindow();

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
  void handleLogout();
  void resetLogoutTimer();
  void toggleTheme();

private:
  VaultController *m_controller;
  QStackedWidget *m_stack;
  QTimer *m_logoutTimer;
  QWidget *m_previousPage;
  bool m_isDarkTheme = true;

  WelcomePage *m_welcome;
  CreateVaultPage *m_create;
  OpenVaultPage *m_open;
  VaultPage *m_vault;
  FolderViewPage *m_folderView;
  EntryEditorPage *m_entryEditor;

  void setupMenuBar();
};
