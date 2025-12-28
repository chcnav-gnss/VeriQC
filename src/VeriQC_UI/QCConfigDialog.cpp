#include "QCConfigDialog.h"
#include "ui_QCConfigDialog.h"
#include "Components/MyMessageBox.h"
#include <QFileDialog>
#include <QListView>

QCConfigDialog::QCConfigDialog(QWidget *parent) :
    MyDialog(parent),
    ui(new Ui::QCConfigDialog)
{
    ui->setupUi(this);

    this->setMinimumWidth(600);

    connect(ui->stationInfoToolButton,&QToolButton::clicked,this,&QCConfigDialog::onStationInfoToolButtonClicked);
    connect(ui->satelliteSysToolButton,&QToolButton::clicked,this,&QCConfigDialog::onSatelliteSysToolButtonClicked);
    connect(ui->thresholdSettingsToolButton,&QToolButton::clicked,this,&QCConfigDialog::onThresholdSettingsToolButtonClicked);

    connect(ui->saveButton,&QPushButton::clicked,this,&QCConfigDialog::onSave);
    connect(ui->cancelButton,&QPushButton::clicked,this,&QCConfigDialog::reject);
    connect(ui->startTimeEnabledCheckBox,&QCheckBox::toggled,ui->startDateTimeEdit,&QDateTimeEdit::setEnabled);
    connect(ui->endTimeEnabledCheckBox,&QCheckBox::toggled,ui->endDateTimeEdit,&QDateTimeEdit::setEnabled);
    connect(ui->workDirComboBox,&QComboBox::currentTextChanged,this,&QCConfigDialog::onWorkDirModeChanged);
    connect(ui->otherWorkDirOpenButton,&QToolButton::clicked,this,&QCConfigDialog::onOtherWorkDirOpenButtonClicked);


    ui->startDateTimeEdit->setDateTime(QDateTime::currentDateTime());
    ui->endDateTimeEdit->setDateTime(QDateTime::currentDateTime());

    QString maxStr = tr("Max");
    QString minStr = tr("Min");
    ui->maxMP1Label->setText(QString("%1MP1(%2)Rms[m]").arg(maxStr).arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP1)));
    ui->maxMP2Label->setText(QString("%1MP2(%2)Rms[m]").arg(maxStr).arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP2)));
    ui->maxMP5Label->setText(QString("%1MP5(%2)Rms[m]").arg(maxStr).arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP5)));
    ui->maxMP6Label->setText(QString("%1MP6(%2)Rms[m]").arg(maxStr).arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP6)));
    ui->maxMP7Label->setText(QString("%1MP7(%2)Rms[m]").arg(maxStr).arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP7)));
    ui->maxMP8Label->setText(QString("%1MP8(%2)Rms[m]").arg(maxStr).arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP8)));

    ui->maxSNR1Label->setText(QString("%1SNR1(%2)[db-Hz]").arg(minStr).arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP1)));
    ui->maxSNR2Label->setText(QString("%1SNR2(%2)[db-Hz]").arg(minStr).arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP2)));
    ui->maxSNR5Label->setText(QString("%1SNR5(%2)[db-Hz]").arg(minStr).arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP5)));
    ui->maxSNR6Label->setText(QString("%1SNR6(%2)[db-Hz]").arg(minStr).arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP6)));
    ui->maxSNR7Label->setText(QString("%1SNR7(%2)[db-Hz]").arg(minStr).arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP7)));
    ui->maxSNR8Label->setText(QString("%1SNR8(%2)[db-Hz]").arg(minStr).arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP8)));

    this->onWorkDirModeChanged();
    ui->satelliteSysToolButton->click();
    ui->thresholdSettingsToolButton->click();
}

QCConfigDialog::~QCConfigDialog()
{
    delete ui;
}

void QCConfigDialog::setConfig(const QCConfigurator::QCConfigData &config, const QCConfigurator::QCCheckConfigData &checkConfig)
{
    m_config = config;
    m_checkConfig = checkConfig;

    // station info
    ui->intervalSpinBox->setValue(m_config.interval);
    ui->elevationMaskSpinBox->setValue(m_config.eleDeg);
    if(m_config.startTime.isValid())
    {
        ui->startDateTimeEdit->setDateTime(m_config.startTime);
        ui->startTimeEnabledCheckBox->setChecked(true);
    }
    if(m_config.endTime.isValid())
    {
        ui->endDateTimeEdit->setDateTime(m_config.endTime);
        ui->endTimeEnabledCheckBox->setCheckable(true);
    }
    ui->sysGPSCheckBox->setChecked(m_config.sysGPSEnable);
    ui->sysBDSCheckBox->setChecked(m_config.sysBDSEnable);
    ui->sysGLOCheckBox->setChecked(m_config.sysGLOEnable);
    ui->sysGALCheckBox->setChecked(m_config.sysGALEnable);
    ui->sysQZSSCheckBox->setChecked(m_config.sysQZSSEnable);

    ui->banPrnEdit->setText(m_config.banPrn);

    QStringList snrGroup;
    for(int item : m_config.snrGroup)
    {
        snrGroup.append(QString::number(item));
    }
    ui->snrGroupEdit->setText(snrGroup.join(';'));

    // QC check settings
    ui->useRateGPSSpinBox->setValue(m_checkConfig.gpsUseRate);
    ui->useRateBDSSpinBox->setValue(m_checkConfig.bdsUseRate);
    ui->useRateGLOSpinBox->setValue(m_checkConfig.gloUseRate);
    ui->useRateGALSpinBox->setValue(m_checkConfig.galUseRate);
    ui->useRateQZSSSpinBox->setValue(m_checkConfig.qzssUseRate);
    ui->cycleJumpRatioSpinBox->setValue(m_checkConfig.CycleJumpRatio);
    ui->maxMP1SpinBox->setValue(m_checkConfig.maxMP[(int)QCResultManager::UiComplexMP1]);
    ui->maxMP2SpinBox->setValue(m_checkConfig.maxMP[(int)QCResultManager::UiComplexMP2]);
    ui->maxMP5SpinBox->setValue(m_checkConfig.maxMP[(int)QCResultManager::UiComplexMP5]);
    ui->maxMP6SpinBox->setValue(m_checkConfig.maxMP[(int)QCResultManager::UiComplexMP6]);
    ui->maxMP7SpinBox->setValue(m_checkConfig.maxMP[(int)QCResultManager::UiComplexMP7]);
    ui->maxMP8SpinBox->setValue(m_checkConfig.maxMP[(int)QCResultManager::UiComplexMP8]);
    ui->minSNR1SpinBox->setValue(m_checkConfig.minSNR[(int)QCResultManager::UiComplexMP1]);
    ui->minSNR2SpinBox->setValue(m_checkConfig.minSNR[(int)QCResultManager::UiComplexMP2]);
    ui->minSNR5SpinBox->setValue(m_checkConfig.minSNR[(int)QCResultManager::UiComplexMP5]);
    ui->minSNR6SpinBox->setValue(m_checkConfig.minSNR[(int)QCResultManager::UiComplexMP6]);
    ui->minSNR7SpinBox->setValue(m_checkConfig.minSNR[(int)QCResultManager::UiComplexMP7]);
    ui->minSNR8SpinBox->setValue(m_checkConfig.minSNR[(int)QCResultManager::UiComplexMP8]);

    //work dir
    switch (config.workDirMode)
    {
    case QCConfigurator::WorkDirModeEnum::SameAsSourceFileDir:
        ui->workDirComboBox->setCurrentIndex(0);
        break;
    case QCConfigurator::WorkDirModeEnum::OtherDir:
        ui->workDirComboBox->setCurrentIndex(2);
        ui->otherWorkDirEdit->setText(config.otherWorkDirPath);
        break;
    default:
        break;
    }
}

const QCConfigurator::QCConfigData &QCConfigDialog::getQCConfig() const
{
    return m_config;
}

const QCConfigurator::QCCheckConfigData &QCConfigDialog::getQCCheckConfig() const
{
    return m_checkConfig;
}

void QCConfigDialog::onSave()
{   // check SNRgroup setting
    QStringList snrGroupStr = ui->snrGroupEdit->text().split(';',QString::SkipEmptyParts);
    QVector<int> snrGroup;
    for(QString item: snrGroupStr)
    {
        bool ok = false;
        snrGroup.append(item.toInt(&ok));
        if(!ok)
        {
            MyMessageBox::critical(this,tr("Error"),tr("Invalid SNRGroup setting"));
            return;
        }
    }

    // station info
    m_config.interval = ui->intervalSpinBox->value();
    m_config.eleDeg = ui->elevationMaskSpinBox->value();
    if(ui->startTimeEnabledCheckBox->isChecked())
    {
        m_config.startTime = ui->startDateTimeEdit->dateTime();
    }
    else
    {
        m_config.startTime = QDateTime();
    }
    if(ui->endTimeEnabledCheckBox->isChecked())
    {
        m_config.endTime = ui->endDateTimeEdit->dateTime();
    }
    else
    {
        m_config.endTime = QDateTime();
    }
    m_config.snrGroup = snrGroup;
    m_config.banPrn = ui->banPrnEdit->text();

    // satellite systems
    m_config.sysGPSEnable = ui->sysGPSCheckBox->isChecked();
    m_config.sysBDSEnable = ui->sysBDSCheckBox->isChecked();
    m_config.sysGLOEnable = ui->sysGLOCheckBox->isChecked();
    m_config.sysGALEnable = ui->sysGALCheckBox->isChecked();
    m_config.sysQZSSEnable = ui->sysQZSSCheckBox->isChecked();

    // QC check settings
    m_checkConfig.gpsUseRate = ui->useRateGPSSpinBox->value();
    m_checkConfig.bdsUseRate = ui->useRateBDSSpinBox->value();
    m_checkConfig.gloUseRate = ui->useRateGLOSpinBox->value();
    m_checkConfig.galUseRate = ui->useRateGALSpinBox->value();
    m_checkConfig.qzssUseRate = ui->useRateQZSSSpinBox->value();
    m_checkConfig.CycleJumpRatio = ui->cycleJumpRatioSpinBox->value();
    m_checkConfig.maxMP[(int)QCResultManager::UiComplexMP1] = ui->maxMP1SpinBox->value();
    m_checkConfig.maxMP[(int)QCResultManager::UiComplexMP2] = ui->maxMP2SpinBox->value();
    m_checkConfig.maxMP[(int)QCResultManager::UiComplexMP5] = ui->maxMP5SpinBox->value();
    m_checkConfig.maxMP[(int)QCResultManager::UiComplexMP6] = ui->maxMP6SpinBox->value();
    m_checkConfig.maxMP[(int)QCResultManager::UiComplexMP7] = ui->maxMP7SpinBox->value();
    m_checkConfig.maxMP[(int)QCResultManager::UiComplexMP8] = ui->maxMP8SpinBox->value();
    m_checkConfig.minSNR[(int)QCResultManager::UiComplexMP1] = ui->minSNR1SpinBox->value();
    m_checkConfig.minSNR[(int)QCResultManager::UiComplexMP2] = ui->minSNR2SpinBox->value();
    m_checkConfig.minSNR[(int)QCResultManager::UiComplexMP5] = ui->minSNR5SpinBox->value();
    m_checkConfig.minSNR[(int)QCResultManager::UiComplexMP6] = ui->minSNR6SpinBox->value();
    m_checkConfig.minSNR[(int)QCResultManager::UiComplexMP7] = ui->minSNR7SpinBox->value();
    m_checkConfig.minSNR[(int)QCResultManager::UiComplexMP8] = ui->minSNR8SpinBox->value();

    //work dir
    if(0 == ui->workDirComboBox->currentIndex())
    {
        m_config.workDirMode = QCConfigurator::WorkDirModeEnum::SameAsSourceFileDir;
    }
    else
    {
        m_config.workDirMode = QCConfigurator::WorkDirModeEnum::OtherDir;
        m_config.otherWorkDirPath = ui->otherWorkDirEdit->text();
    }

    this->accept();
}

void QCConfigDialog::onStationInfoToolButtonClicked(bool checked)
{
    if(checked)
    {
        ui->stationInfoToolButton->setArrowType(Qt::DownArrow);
    }
    else
    {
        ui->stationInfoToolButton->setArrowType(Qt::RightArrow);
    }
    ui->stationInfoWidget->setVisible(checked);
    this->adjustSize();
}

void QCConfigDialog::onSatelliteSysToolButtonClicked(bool checked)
{
    if(checked)
    {
        ui->satelliteSysToolButton->setArrowType(Qt::DownArrow);
    }
    else
    {
        ui->satelliteSysToolButton->setArrowType(Qt::RightArrow);
    }
    ui->satelliteSystemWidget->setVisible(checked);
    this->adjustSize();
}

void QCConfigDialog::onThresholdSettingsToolButtonClicked(bool checked)
{
    if(checked)
    {
        ui->thresholdSettingsToolButton->setArrowType(Qt::DownArrow);
    }
    else
    {
        ui->thresholdSettingsToolButton->setArrowType(Qt::RightArrow);
    }
    ui->thresholdSettingsWidget->setVisible(checked);
    this->adjustSize();
}

void QCConfigDialog::onWorkDirModeChanged()
{
    bool isOtherDir = ui->workDirComboBox->currentIndex() == 1; // is other work dir
    ui->otherWorkDirEdit->setVisible(isOtherDir);
    ui->otherWorkDirOpenButton->setVisible(isOtherDir);
}

void QCConfigDialog::onOtherWorkDirOpenButtonClicked()
{
    QString path = QFileDialog::getExistingDirectory(this);
    if(path.isEmpty())
    {
        return;
    }
    ui->otherWorkDirEdit->setText(path);
}
