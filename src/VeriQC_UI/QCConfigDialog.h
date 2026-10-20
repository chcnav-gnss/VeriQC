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
#include <QButtonGroup>

class QCheckBox;
class QDateTimeEdit;

namespace Ui {
class QCConfigDialog;
}

class QCConfigDialog : public MyDialog
{
    Q_OBJECT

public:
    explicit QCConfigDialog(QWidget *parent = nullptr);
    ~QCConfigDialog();
    /**< Fill the dialog from the stored config. @p obsStartTime/@p obsEndTime are
     *   the current-row object's *real* observation times (only available after QC
     *   has run); when a time-range checkbox is unchecked, the corresponding edit
     *   shows this real time read-only (or "--" when unavailable) instead of the
     *   config value. Invalid by default for backward compatibility. */
    void setConfig(const QCConfigurator::QCConfigData & config,
                   const QCConfigurator::QCCheckConfigData & checkConfig,
                   const QDateTime & obsStartTime = QDateTime(),
                   const QDateTime & obsEndTime = QDateTime());
    const QCConfigurator::QCConfigData &getQCConfig()const;
    const QCConfigurator::QCCheckConfigData &getQCCheckConfig()const;
protected:
    void onSave();
    void onStationInfoToolButtonClicked(bool checked);
    void onSatelliteSysToolButtonClicked(bool checked);
    void onThresholdSettingsToolButtonClicked(bool checked);
    void onWorkDirModeChanged();
    void onOtherWorkDirOpenButtonClicked();
    /**< Toggle the ephemeris file edit/browse controls based on the selected
     *   radio (enabled only when "Specify ephemeris file" is checked). */
    void onEphModeChanged();
    /**< Open a single RINEX nav file picker and fill the path edit. */
    void onEphBrowseClicked();
private:
    Ui::QCConfigDialog *ui;
    QCConfigurator::QCConfigData m_config;
    QCConfigurator::QCCheckConfigData m_checkConfig;
    QButtonGroup *m_ephButtonGroup;
    /**< Populate one time-range row. If @p configTime is valid, the checkbox is
     *   checked and the edit shows @p configTime (user-editable). Otherwise the
     *   checkbox is cleared and the edit is read-only, showing @p obsTime when
     *   valid or the "--" placeholder (minimumDateTime()) when not. */
    void applyTimeRange(QCheckBox *checkBox,
                        QDateTimeEdit *edit,
                        const QDateTime &configTime,
                        const QDateTime &obsTime);
    /**< Check the radio button corresponding to @p mode (Default for the
     *   default/unknown case). Centralises the enum->radio mapping shared by
     *   construction and setConfig(). */
    void setEphemerisModeRadio(QCConfigurator::QCConfigData::EphemerisModeEnum mode);
    /**< Read the currently checked radio back into an ephemeris-mode enum
     *   (Default when none of the mode-specific radios is checked). */
    QCConfigurator::QCConfigData::EphemerisModeEnum getEphemerisModeFromRadio() const;
};

#endif // QCCONFIGDIALOG_H
