#include "CNewVersionDescDialog.h"
#include "ui_CNewVersionDescDialog.h"

CNewVersionDescDialog::CNewVersionDescDialog(QString newVersion, QString desc, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CNewVersionDescDialog)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("UpgradeDesc"));
    ui->textBrowser->setText(desc);
    ui->versionLabel->setText(newVersion);

    connect(ui->upgradeButton, &QPushButton::clicked, this, &CNewVersionDescDialog::accept);
    connect(ui->cancelButton, &QPushButton::clicked, this, &CNewVersionDescDialog::reject);
}

CNewVersionDescDialog::~CNewVersionDescDialog()
{
    delete ui;
}
