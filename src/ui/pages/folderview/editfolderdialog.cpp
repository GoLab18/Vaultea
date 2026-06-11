#include "editfolderdialog.h"
#include "ui_editfolderdialog.h"

EditFolderDialog::EditFolderDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::EditFolderDialog) {
  ui->setupUi(this);

  connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

EditFolderDialog::~EditFolderDialog() { delete ui; }

void EditFolderDialog::setInitialData(const QString &name,
                                      const QString &description) {
  ui->nameEdit->setText(name);
  ui->descEdit->setText(description);
}

QString EditFolderDialog::name() const { return ui->nameEdit->text(); }

QString EditFolderDialog::description() const { return ui->descEdit->text(); }
