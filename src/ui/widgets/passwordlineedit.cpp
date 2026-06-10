#include "passwordlineedit.h"
#include "../theme/thememanager.h"

#include <QAction>

PasswordLineEdit::PasswordLineEdit(QWidget *parent) : QLineEdit(parent) {
  setEchoMode(QLineEdit::Password);

  QColor iconColor = ThemeManager::getMutedColor();
  QIcon eyeOffIcon =
      ThemeManager::colorizedIcon(":/assets/icons/eye-off.svg", iconColor, 20);

  m_toggleAction = addAction(eyeOffIcon, QLineEdit::TrailingPosition);

  connect(m_toggleAction, &QAction::triggered, this,
          &PasswordLineEdit::toggleVisibility);
}

void PasswordLineEdit::toggleVisibility() {
  m_visible = !m_visible;
  setEchoMode(m_visible ? QLineEdit::Normal : QLineEdit::Password);

  QColor iconColor = ThemeManager::getMutedColor();
  if (m_visible) {
    m_toggleAction->setIcon(
        ThemeManager::colorizedIcon(":/assets/icons/eye.svg", iconColor, 20));
  } else {
    m_toggleAction->setIcon(ThemeManager::colorizedIcon(
        ":/assets/icons/eye-off.svg", iconColor, 20));
  }
}
