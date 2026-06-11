#pragma once

#include <QWidget>

class QLineEdit;
class QPushButton;
class PasswordLineEdit;

class CreateVaultPage : public QWidget {
  Q_OBJECT

public:
  explicit CreateVaultPage(QWidget *parent = nullptr);

  void clearFields();

signals:
  void createRequested(QString path, QString password);
  void backRequested();

private:
  QLineEdit *m_dirPath;
  QLineEdit *m_fileName;
  PasswordLineEdit *m_password;
  PasswordLineEdit *m_confirm;
};
