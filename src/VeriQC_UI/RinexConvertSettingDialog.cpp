#include "RinexConvertSettingDialog.h"
#include "ui_RinexConvertSettingDialog.h"

RinexConvertSettingDialog::RinexConvertSettingDialog(QWidget *parent) :
    MyDialog(parent),
    ui(new Ui::RinexConvertSettingDialog)
{
    ui->setupUi(this);

    ui->rinexVersionComboBox->addItems(QStringList()<<"2.11"<<"3.01"<<"3.02"<<"3.03"<<"3.04"<<"4.00");

    /**< Sample interval supports sub-second values (e.g. 0.005s for 200Hz);
     *  0 means "derive from the raw file". QDoubleSpinBox defaults to 2
     *  decimals and a 0..99.99 range, which is too coarse and too narrow. */
    ui->intervalSpinBox->setDecimals(3);
    ui->intervalSpinBox->setMinimum(0.0);
    ui->intervalSpinBox->setMaximum(3600.0);

    connect(ui->saveButton,&QPushButton::clicked,this,&RinexConvertSettingDialog::onSave);
    connect(ui->cancelButton,&QPushButton::clicked,this,&RinexConvertSettingDialog::reject);

}

RinexConvertSettingDialog::~RinexConvertSettingDialog()
{
    delete ui;
}

void RinexConvertSettingDialog::setRinexConvertSettings(const QCConfigurator::RinexConvertSettings &settings)
{
    ui->rinexVersionComboBox->setCurrentText(QString::number(settings.rinexVersion/100.0,'f',2));
    ui->intervalSpinBox->setValue(settings.interval);
    if(settings.sysGPSEnable)
    {
        ui->gpsTrueRadioButton->setChecked(true);
    }
    else
    {
        ui->gpsFalseRadioButton->setChecked(true);
    }

    if(settings.sysBDSEnable)
    {
        ui->bdsTrueRadioButton->setChecked(true);
    }
    else
    {
        ui->bdsFalseRadioButton->setChecked(true);
    }

    if(settings.sysGLOEnable)
    {
        ui->gloTrueRadioButton->setChecked(true);
    }
    else
    {
        ui->gloFalseRadioButton->setChecked(true);
    }

    if(settings.sysGALEnable)
    {
        ui->galTrueRadioButton->setChecked(true);
    }
    else
    {
        ui->galFalseRadioButton->setChecked(true);
    }

    if(settings.sysQZSSEnable)
    {
        ui->qzssTrueRadioButton->setChecked(true);
    }
    else
    {
        ui->qzssFalseRadioButton->setChecked(true);
    }
}

const QCConfigurator::RinexConvertSettings &RinexConvertSettingDialog::getRinexConvertSettings() const
{
    return m_settings;
}

void RinexConvertSettingDialog::onSave()
{
    m_settings.rinexVersion = (unsigned int)(ui->rinexVersionComboBox->currentText().toDouble()*100 + 0.1);
    m_settings.interval = ui->intervalSpinBox->value();
    m_settings.sysGPSEnable = ui->gpsTrueRadioButton->isChecked();
    m_settings.sysBDSEnable = ui->bdsTrueRadioButton->isChecked();
    m_settings.sysGLOEnable = ui->gloTrueRadioButton->isChecked();
    m_settings.sysGALEnable = ui->galTrueRadioButton->isChecked();
    m_settings.sysQZSSEnable = ui->qzssTrueRadioButton->isChecked();

    this->accept();
}
