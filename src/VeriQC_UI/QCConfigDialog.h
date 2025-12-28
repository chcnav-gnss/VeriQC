/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Module
 * *-
 * @file QCConfigDialog.h
 * @author CHC
 * @date 2025-09-28
 * @brief Dilaog for config QC 
 * 
**************************************************************************/
#ifndef QCCONFIGDIALOG_H
#define QCCONFIGDIALOG_H

#include "Components/MyDialog.h"
#include "QCConfigurator.h"

namespace Ui {
class QCConfigDialog;
}

class QCConfigDialog : public MyDialog
{
    Q_OBJECT

public:
    explicit QCConfigDialog(QWidget *parent = nullptr);
    ~QCConfigDialog();
    void setConfig(const QCConfigurator::QCConfigData & config, const QCConfigurator::QCCheckConfigData & checkConfig);
    const QCConfigurator::QCConfigData &getQCConfig()const;
    const QCConfigurator::QCCheckConfigData &getQCCheckConfig()const;
protected:
    void onSave();
    void onStationInfoToolButtonClicked(bool checked);
    void onSatelliteSysToolButtonClicked(bool checked);
    void onThresholdSettingsToolButtonClicked(bool checked);
    void onWorkDirModeChanged();
    void onOtherWorkDirOpenButtonClicked();
private:
    Ui::QCConfigDialog *ui;
    QCConfigurator::QCConfigData m_config;
    QCConfigurator::QCCheckConfigData m_checkConfig;
};

#endif // QCCONFIGDIALOG_H
