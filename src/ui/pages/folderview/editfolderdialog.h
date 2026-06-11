#pragma once

#include <QDialog>
#include <QString>

namespace Ui {
class EditFolderDialog;
}

class EditFolderDialog : public QDialog {
  Q_OBJECT

public:
  explicit EditFolderDialog(QWidget *parent = nullptr);
  ~EditFolderDialog();

  void setInitialData(const QString &name, const QString &description);

  QString name() const;
  QString description() const;

private:
  Ui::EditFolderDialog *ui;
};
