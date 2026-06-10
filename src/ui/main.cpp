#include "ui/windows/mainwindow.h"
#include "ui/theme/thememanager.h"

#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  ThemeManager::applyTheme(AppTheme::Dark);

  MainWindow window;
  window.resize(1000, 700);
  window.setWindowTitle("Vaultea");
  window.show();

  return app.exec();
}
