#pragma once

#include <QWidget>

class QLineEdit;
class PasswordLineEdit;

class OpenVaultPage : public QWidget {
  Q_OBJECT

public:
  explicit OpenVaultPage(QWidget *parent = nullptr);

signals:
  void openRequested(QString path, QString password);
  void backRequested();

private:
  QLineEdit *m_path;
  PasswordLineEdit *m_password;
};
