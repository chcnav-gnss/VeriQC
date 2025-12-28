/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Rinex Convert Module
 * *-
 * @file RinexConvertSettingDialog.h
 * @author CHC
 * @date 2025-09-28
 * @brief dialog for config Rinex convert
 * 
**************************************************************************/
#ifndef RINEXCONVERTSETTINGDIALOG_H
#define RINEXCONVERTSETTINGDIALOG_H

#include "Components/MyDialog.h"
#include "AppSettingsManagement.h"

namespace Ui {
class RinexConvertSettingDialog;
}

class RinexConvertSettingDialog : public MyDialog
{
    Q_OBJECT

public:
    explicit RinexConvertSettingDialog(QWidget *parent = nullptr);
    ~RinexConvertSettingDialog();
   void setRinexConvertSettings(const QCConfigurator::RinexConvertSettings &settings);
   const QCConfigurator::RinexConvertSettings &getRinexConvertSettings()const;
protected:
    void onSave();
    void onStartDateTimeChanged();
    void onEndDateTimeChanged();
private:
    Ui::RinexConvertSettingDialog *ui;
    QCConfigurator::RinexConvertSettings m_settings;
};

#endif // RINEXCONVERTSETTINGDIALOG_H
