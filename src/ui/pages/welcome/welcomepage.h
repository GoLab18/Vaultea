#pragma once

#include <QWidget>

namespace Ui {
class WelcomePage;
}

class WelcomePage : public QWidget {
  Q_OBJECT

public:
  explicit WelcomePage(QWidget *parent = nullptr);
  ~WelcomePage();

signals:
  void createRequested();
  void openRequested();

private:
  Ui::WelcomePage *ui;
};
