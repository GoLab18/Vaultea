#include "thememanager.h"

#include <QColor>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QStyleFactory>
#include <qstyle.h>

AppTheme ThemeManager::currentTheme = AppTheme::Dark;

QColor ThemeManager::getAccentColor() { return QColor("#2a82da"); }

QColor ThemeManager::getTextColor() {
  return currentTheme == AppTheme::Dark ? Qt::white : Qt::black;
}

QColor ThemeManager::getMutedColor() {
  return currentTheme == AppTheme::Dark ? QColor(150, 150, 150)
                                        : QColor(100, 100, 100);
}

QIcon ThemeManager::colorizedIcon(const QString &iconPath, const QColor &color,
                                  int size) {
  QIcon originalIcon(iconPath);
  QPixmap pixmap = originalIcon.pixmap(size, size);

  if (pixmap.isNull())
    return QIcon();

  QPainter painter(&pixmap);
  painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  painter.fillRect(pixmap.rect(), color);
  painter.end();

  return QIcon(pixmap);
}

void ThemeManager::applyTheme(AppTheme theme) {
  currentTheme = theme;
  qApp->setStyle(QStyleFactory::create("Fusion"));

  switch (theme) {
  case AppTheme::Dark:
    setDarkTheme();
    break;
  case AppTheme::Light:
    setLightTheme();
    break;
  }
}

void ThemeManager::setDarkTheme() {
  QPalette darkPalette;
  darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
  darkPalette.setColor(QPalette::WindowText, Qt::white);
  darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
  darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
  darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
  darkPalette.setColor(QPalette::ToolTipText, Qt::white);
  darkPalette.setColor(QPalette::Text, Qt::white);
  darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
  darkPalette.setColor(QPalette::ButtonText, Qt::white);
  darkPalette.setColor(QPalette::BrightText, Qt::red);
  darkPalette.setColor(QPalette::Link, getAccentColor());
  darkPalette.setColor(QPalette::Highlight, getAccentColor());
  darkPalette.setColor(QPalette::HighlightedText, Qt::black);
  darkPalette.setColor(QPalette::PlaceholderText, Qt::gray);

  qApp->setPalette(darkPalette);
}

void ThemeManager::setLightTheme() {
  QPalette lightPalette = qApp->style()->standardPalette();
  lightPalette.setColor(QPalette::Link, getAccentColor());
  lightPalette.setColor(QPalette::Highlight, getAccentColor());
  qApp->setPalette(lightPalette);
}
