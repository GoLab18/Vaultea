#pragma once

#include "models/VaultEntry.h"

#include <QApplication>
#include <QColor>
#include <QIcon>

enum class AppTheme { Dark, Light };

class ThemeManager {
public:
  static void applyTheme(AppTheme theme);

  static QColor getAccentColor();
  static QColor getTextColor();
  static QColor getMutedColor();

  static QIcon colorizedIcon(const QString &iconPath, const QColor &color,
                             int size = 24);

  static QIcon iconForEntryType(EntryType type);

private:
  static AppTheme currentTheme;

  static void setDarkTheme();
  static void setLightTheme();
};
