/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Module
 * *-
 * @file AppSettingsManagement.h
 * @author CHC
 * @date 2025-09-28
 * @brief management all app settings
 * 
**************************************************************************/
#ifndef APPSETTINGSMANAGEMENT_H
#define APPSETTINGSMANAGEMENT_H


#include <QDateTime>
#include <QSettings>
#include <QLocale>
#include "QCConfigurator.h"

class AppSettingsManagement
{
public:
    static AppSettingsManagement *getInstance();
    void saveRinexConvertSettings(const QCConfigurator::RinexConvertSettings &settings);
    const QCConfigurator::RinexConvertSettings &getRinexConvertSettings()const;
    void saveDefaultQCConfig(const QCConfigurator::QCConfigData& config, const QCConfigurator::QCCheckConfigData& checkConfig);
    const QCConfigurator::QCConfigData &getDefaultQCConfig()const;
    const QCConfigurator::QCCheckConfigData &getDefaultQCCheckConfig()const;
    void saveLanguage(QLocale::Language language);
    QLocale::Language getLanguage()const;
    void setUpgradePlugArgs(const QStringList &args);
    const QStringList &getUpgradePlugArgs()const;
private:
    void loadRinexConvertSettings();
    void loadDefaultQCConfig();
    void loadCommonSettings();
    QString rinexConvertSettingsFilePath();
    QString defaultQCConfigFilePath();
    QString commonSettingsFilePath();
    void serializeArray(QSettings& config, const QString& prefix, const float* array, int size);
    void deserializeArray(QSettings& config, const QString& prefix, float* array, int size, float *defaultValueArray);
private:
    explicit AppSettingsManagement();
    QCConfigurator::RinexConvertSettings m_rinexConvertSettings;
    QCConfigurator::QCConfigData m_defaultQCConfigData;
    QCConfigurator::QCCheckConfigData m_defaultQCCheckConfigData;
    QLocale::Language m_language = QLocale::Language::English;
    QStringList m_upgradePlugArgs;
};

#endif // APPSETTINGSMANAGEMENT_H
