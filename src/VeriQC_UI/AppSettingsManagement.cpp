#include "AppSettingsManagement.h"
#include <QDir>
#include <QCoreApplication>

static QCConfigurator::QCCheckConfigData s_defaultCheckConfigData = {
    95,95,95,95,95,
    400,
    {0.5,0.5,0.5,0.5,0.5,0.5},
    {48,36,45,45,45,45}
};

AppSettingsManagement *AppSettingsManagement::getInstance()
{
    static AppSettingsManagement instance;
    return &instance;
}

void AppSettingsManagement::saveRinexConvertSettings(const QCConfigurator::RinexConvertSettings &settings)
{
    QSettings config(rinexConvertSettingsFilePath(), QSettings::IniFormat);

    config.beginGroup("RinexSettings");
    config.setValue("rinexVersion", settings.rinexVersion);
    config.setValue("interval", settings.interval);
    config.setValue("timePeriodSplit", settings.timePeriodSplit);
    config.setValue("startSplitTime", settings.startSplitTime.toString(Qt::ISODate));
    config.setValue("endSplitTime", settings.endSplitTime.toString(Qt::ISODate));
    config.setValue("sysGPSEnable", settings.sysGPSEnable);
    config.setValue("sysBDSEnable", settings.sysBDSEnable);
    config.setValue("sysGLOEnable", settings.sysGLOEnable);
    config.setValue("sysGALEnable", settings.sysGALEnable);
    config.setValue("sysQZSSEnable", settings.sysQZSSEnable);
    config.endGroup();

    m_rinexConvertSettings = settings;
}

const QCConfigurator::RinexConvertSettings &AppSettingsManagement::getRinexConvertSettings() const
{
    return m_rinexConvertSettings;
}

void AppSettingsManagement::saveDefaultQCConfig(const QCConfigurator::QCConfigData &config, const QCConfigurator::QCCheckConfigData &checkConfig)
{
    QSettings settings(defaultQCConfigFilePath(), QSettings::IniFormat);

    // QC config
    settings.beginGroup("QCConfig");
    settings.setValue("interval", config.interval);
    settings.setValue("eleDeg", config.eleDeg);
    settings.setValue("sysGPSEnable", config.sysGPSEnable);
    settings.setValue("sysBDSEnable", config.sysBDSEnable);
    settings.setValue("sysGLOEnable", config.sysGLOEnable);
    settings.setValue("sysGALEnable", config.sysGALEnable);
    settings.setValue("sysQZSSEnable", config.sysQZSSEnable);
    settings.setValue("startTime", config.startTime.toString(Qt::ISODate));
    settings.setValue("endTime", config.endTime.toString(Qt::ISODate));
    settings.setValue("banPrn", config.banPrn);
    QStringList snrGroup;
    for(auto snr : config.snrGroup)
    {
        snrGroup.append(QString::number(snr));
    }
    settings.setValue("snrGroup", QVariant::fromValue(snrGroup.join(",")));
    settings.setValue("workDirMode", config.workDirMode);
    settings.setValue("otherWorkDir", config.otherWorkDirPath);
    settings.endGroup();

    // QC Check Config
    settings.beginGroup("QCCheckConfig");
    settings.setValue("gpsUseRate", checkConfig.gpsUseRate);
    settings.setValue("bdsUseRate", checkConfig.bdsUseRate);
    settings.setValue("gloUseRate", checkConfig.gloUseRate);
    settings.setValue("galUseRate", checkConfig.galUseRate);
    settings.setValue("qzssUseRate", checkConfig.qzssUseRate);
    settings.setValue("CycleJumpRatio", checkConfig.CycleJumpRatio);
    serializeArray(settings, "maxMP", checkConfig.maxMP, UI_COMPLEX_MP_MAX_NUM);
    serializeArray(settings, "minSNR", checkConfig.minSNR, UI_COMPLEX_MP_MAX_NUM);
    settings.endGroup();

    m_defaultQCConfigData = config;
    m_defaultQCCheckConfigData = checkConfig;
}

const QCConfigurator::QCConfigData &AppSettingsManagement::getDefaultQCConfig() const
{
    return m_defaultQCConfigData;
}

const QCConfigurator::QCCheckConfigData &AppSettingsManagement::getDefaultQCCheckConfig() const
{
    return m_defaultQCCheckConfigData;
}

void AppSettingsManagement::saveLanguage(QLocale::Language language)
{
    QSettings settings(commonSettingsFilePath(), QSettings::IniFormat);
    settings.setValue("Language",language);

    m_language = language;
}

QLocale::Language AppSettingsManagement::getLanguage() const
{
    return m_language;
}

void AppSettingsManagement::setUpgradePlugArgs(const QStringList &args)
{
    m_upgradePlugArgs = args;
}

const QStringList &AppSettingsManagement::getUpgradePlugArgs() const
{
    return m_upgradePlugArgs;
}

void AppSettingsManagement::loadRinexConvertSettings()
{
    QSettings config(rinexConvertSettingsFilePath(), QSettings::IniFormat);
    QCConfigurator::RinexConvertSettings settings;

    config.beginGroup("RinexSettings");
    settings.rinexVersion = config.value("rinexVersion", 304).toUInt();
    settings.interval = config.value("interval", 0).toUInt();
    settings.timePeriodSplit = config.value("timePeriodSplit", false).toBool();
    settings.startSplitTime = QDateTime::fromString(
        config.value("startSplitTime").toString(), Qt::ISODate);
    settings.endSplitTime = QDateTime::fromString(
        config.value("endSplitTime").toString(), Qt::ISODate);
    settings.sysGPSEnable = config.value("sysGPSEnable", true).toBool();
    settings.sysBDSEnable = config.value("sysBDSEnable", true).toBool();
    settings.sysGLOEnable = config.value("sysGLOEnable", true).toBool();
    settings.sysGALEnable = config.value("sysGALEnable", true).toBool();
    settings.sysQZSSEnable = config.value("sysQZSSEnable", true).toBool();
    config.endGroup();

    m_rinexConvertSettings = settings;
}

void AppSettingsManagement::loadDefaultQCConfig()
{
    QSettings settings(defaultQCConfigFilePath(), QSettings::IniFormat);
    QCConfigurator::QCConfigData config = QCConfigurator::initConfig();
    QCConfigurator::QCCheckConfigData checkConfig = s_defaultCheckConfigData;

    // QC Config
    settings.beginGroup("QCConfig");
    config.interval = settings.value("interval", config.interval).toFloat();
    config.eleDeg = settings.value("eleDeg", config.eleDeg).toUInt();
    config.sysGPSEnable = settings.value("sysGPSEnable",config.sysGPSEnable).toBool();
    config.sysBDSEnable = settings.value("sysBDSEnable",config.sysBDSEnable).toBool();
    config.sysGLOEnable = settings.value("sysGLOEnable",config.sysGLOEnable).toBool();
    config.sysGALEnable = settings.value("sysGALEnable",config.sysGALEnable).toBool();
    config.sysQZSSEnable = settings.value("sysQZSSEnable",config.sysQZSSEnable).toBool();
    config.startTime = QDateTime::fromString(settings.value("startTime").toString(), Qt::ISODate);
    config.endTime = QDateTime::fromString(settings.value("endTime").toString(), Qt::ISODate);
    config.banPrn = settings.value("banPrn").toString();
    QString snrGroupStr = settings.value("snrGroup").toString();
    if (!snrGroupStr.isEmpty()) {
        config.snrGroup.clear();
        QStringList items = snrGroupStr.split(',',QString::SkipEmptyParts);
        for(const QString& item : items) {
            config.snrGroup.append(item.toInt());
        }
    }
    int workDirMode = settings.value("workDirMode", 0).toInt();
    if(workDirMode >= 0 && workDirMode <= QCConfigurator::WorkDirModeEnum::OtherDir)
    {
        config.workDirMode = (QCConfigurator::WorkDirModeEnum)workDirMode;
    }
    config.otherWorkDirPath = settings.value("otherWorkDir").toString();
    settings.endGroup();

    // QC Check Config
    settings.beginGroup("QCCheckConfig");
    checkConfig.gpsUseRate = settings.value("gpsUseRate", s_defaultCheckConfigData.gpsUseRate).toFloat();
    checkConfig.bdsUseRate = settings.value("bdsUseRate", s_defaultCheckConfigData.bdsUseRate).toFloat();
    checkConfig.gloUseRate = settings.value("gloUseRate", s_defaultCheckConfigData.gloUseRate).toFloat();
    checkConfig.galUseRate = settings.value("galUseRate", s_defaultCheckConfigData.galUseRate).toFloat();
    checkConfig.qzssUseRate = settings.value("qzssUseRate", s_defaultCheckConfigData.qzssUseRate).toFloat();
    checkConfig.CycleJumpRatio = settings.value("CycleJumpRatio", s_defaultCheckConfigData.CycleJumpRatio).toUInt();
    deserializeArray(settings, "maxMP", checkConfig.maxMP, UI_COMPLEX_MP_MAX_NUM,s_defaultCheckConfigData.maxMP);
    deserializeArray(settings, "minSNR", checkConfig.minSNR, UI_COMPLEX_MP_MAX_NUM,s_defaultCheckConfigData.minSNR);
    settings.endGroup();

    m_defaultQCConfigData = config;
    m_defaultQCCheckConfigData = checkConfig;
}

void AppSettingsManagement::loadCommonSettings()
{
    QSettings settings(commonSettingsFilePath(), QSettings::IniFormat);
    m_language = settings.value("Language", QLocale::system().language()).value<QLocale::Language>();
}

QString AppSettingsManagement::rinexConvertSettingsFilePath()
{
    QDir configDir(QCoreApplication::applicationDirPath() + "/config");
    if (!configDir.exists())
    {
        configDir.mkpath(".");
    }
    return configDir.filePath("RinexConvertSettings.ini");
}

QString AppSettingsManagement::defaultQCConfigFilePath()
{
    QDir configDir(QCoreApplication::applicationDirPath() + "/config");
    if (!configDir.exists())
    {
        configDir.mkpath(".");
    }
    return configDir.filePath("DefaultQCConfig.ini");
}

QString AppSettingsManagement::commonSettingsFilePath()
{
    QDir configDir(QCoreApplication::applicationDirPath() + "/config");
    if (!configDir.exists())
    {
        configDir.mkpath(".");
    }
    return configDir.filePath("CommonSettings.ini");
}

void AppSettingsManagement::serializeArray(QSettings &config, const QString &prefix, const float *array, int size)
{
    for(int i = 0; i < size; ++i)
    {
        config.setValue(QString("%1_%2").arg(prefix).arg(i), array[i]);
    }
}

void AppSettingsManagement::deserializeArray(QSettings &config, const QString &prefix, float *array, int size, float *defaultValueArray)
{
    for(int i = 0; i < size; ++i)
    {
        array[i] = config.value(QString("%1_%2").arg(prefix).arg(i), defaultValueArray[i]).toFloat();
    }
}

AppSettingsManagement::AppSettingsManagement()
{
    m_defaultQCConfigData = QCConfigurator::initConfig();
    loadRinexConvertSettings();
    loadDefaultQCConfig();
    loadCommonSettings();
}
