#pragma once

#include <QLineEdit>

class QAction;

class PasswordLineEdit : public QLineEdit {
  Q_OBJECT

public:
  explicit PasswordLineEdit(QWidget *parent = nullptr);

private:
  QAction *m_toggleAction;

  bool m_visible = false;

  void toggleVisibility();
};
