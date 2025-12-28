#include "RinexConvertSettingDialog.h"
#include "ui_RinexConvertSettingDialog.h"

RinexConvertSettingDialog::RinexConvertSettingDialog(QWidget *parent) :
    MyDialog(parent),
    ui(new Ui::RinexConvertSettingDialog)
{
    ui->setupUi(this);

    ui->rinexVersionComboBox->addItems(QStringList()<<"2.11"<<"3.01"<<"3.02"<<"3.03"<<"3.04"<<"4.00");

    connect(ui->saveButton,&QPushButton::clicked,this,&RinexConvertSettingDialog::onSave);
    connect(ui->cancelButton,&QPushButton::clicked,this,&RinexConvertSettingDialog::reject);
    connect(ui->startDateTimeEdit,&QDateTimeEdit::dateTimeChanged,this,&RinexConvertSettingDialog::onStartDateTimeChanged);
    connect(ui->endDateTimeEdit,&QDateTimeEdit::dateTimeChanged,this,&RinexConvertSettingDialog::onEndDateTimeChanged);

}

RinexConvertSettingDialog::~RinexConvertSettingDialog()
{
    delete ui;
}

void RinexConvertSettingDialog::setRinexConvertSettings(const QCConfigurator::RinexConvertSettings &settings)
{
    ui->rinexVersionComboBox->setCurrentText(QString::number(settings.rinexVersion/100.0,'f',2));
    ui->intervalSpinBox->setValue(settings.interval);
    if(settings.timePeriodSplit)
    {
        ui->sepFileTrueRadioButton->setChecked(true);
    }
    else
    {
        ui->sepFileFalseRadioButton->setChecked(true);
    }
    ui->startDateTimeEdit->setDateTime(settings.startSplitTime);
    ui->endDateTimeEdit->setDateTime(settings.endSplitTime);
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
    m_settings.timePeriodSplit = ui->sepFileTrueRadioButton->isChecked();
    if(m_settings.timePeriodSplit)
    {
        m_settings.startSplitTime = ui->startDateTimeEdit->dateTime();
        m_settings.endSplitTime = ui->endDateTimeEdit->dateTime();
    }
    m_settings.sysGPSEnable = ui->gpsTrueRadioButton->isChecked();
    m_settings.sysBDSEnable = ui->bdsTrueRadioButton->isChecked();
    m_settings.sysGLOEnable = ui->gloTrueRadioButton->isChecked();
    m_settings.sysGALEnable = ui->galTrueRadioButton->isChecked();
    m_settings.sysQZSSEnable = ui->qzssTrueRadioButton->isChecked();

    this->accept();
}

void RinexConvertSettingDialog::onStartDateTimeChanged()
{
    if(ui->endDateTimeEdit->dateTime() < ui->startDateTimeEdit->dateTime())
    {
        ui->endDateTimeEdit->setDateTime(ui->startDateTimeEdit->dateTime().addSecs(1));
    }
}

void RinexConvertSettingDialog::onEndDateTimeChanged()
{
    if(ui->startDateTimeEdit->dateTime() > ui->endDateTimeEdit->dateTime())
    {
        ui->startDateTimeEdit->setDateTime(ui->endDateTimeEdit->dateTime().addSecs(-1));
    }
}
