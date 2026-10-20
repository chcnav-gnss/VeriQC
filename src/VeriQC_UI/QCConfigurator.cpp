#include "QCConfigurator.h"
#include <QMap>
#include <QFileInfo>
#include <QDir>
#include <QComboBox>
#include <QRegularExpression>
#include <QStringList>
#include <vector>
#include "QCResultManager.h"
#include "AppSettingsManagement.h"
#include "PathUtils.h"

#define GNSS_STRFMT_RTCM2			0		/**< stream format: RTCM 2 */
#define GNSS_STRFMT_RTCM3			1		/**< stream format: RTCM 3 */
#define GNSS_STRFMT_RINEX			11		/**< stream format: RINEX */
#define GNSS_STRFMT_HRCX			37		/**< stream format: HRCX */
#define GNSS_STRFMT_HRC				38		/**< stream format: HRC */
#define GNSS_STRFMT_RAWMSGA			51		/**< RAWMSGA */
#define GNSS_STRFMT_RAWMSGB			52		/**< RAWMSGB */
#define GNSS_STRFMT_BASEOBSA		53		/**< BASEOBSA */
#define GNSS_STRFMT_BASEOBSB		54		/**< BASEOBSB */
#define GNSS_STRFMT_KMD_RAWMSGA		101		/**< KMD_RAWMSGA */
#define GNSS_STRFMT_KMD_RAWMSGB		102		/**< KMD_RAWMSGB */
#define GNSS_STRFMT_RTKLOGA			103		/**< RTKLOGA */
#define GNSS_STRFMT_RTKLOGB			104		/**< RTKLOGB */
#define GNSS_STRFMT_CLAS			105		/**< CLAS */

#define DATA_OUT_TYPE_RINEX         0       /**< output RINEX */


static QC_TIME_T dateTimeToQCTime(const QDateTime &dateTime)
{
    QC_TIME_T qcTime = {};
    QString dateTimeStr = dateTime.toString("yyyy-MM-dd HH:mm:ss");
    int ep[6] = { 0 };
    double epo[6] = { 0.0 };
    int ScanResult = sscanf(dateTimeStr.toLocal8Bit().data(), "%4d-%02d-%02d %02d:%02d:%02d", ep, ep + 1, ep + 2, ep + 3, ep + 4, ep + 5);
    if (ScanResult == 6)
    {
        for (int i = 0; i < 6; i++) epo[i] = ep[i];
        qcTime = Epoch2QCTime(epo);
    }
    return qcTime;
}

QCConfigurator::QCConfigurator()
{
    m_checkConfig = AppSettingsManagement::getInstance()->getDefaultQCCheckConfig();
    m_config = AppSettingsManagement::getInstance()->getDefaultQCConfig();
}

QString QCConfigurator::getFileName() const
{
    return QFileInfo(m_rawFileInfo.rawFilePath).fileName();
}

void QCConfigurator::setRawFileInfo(const QCConfigRawFileInfo &info)
{
    m_rawFileInfo = info;
    /**< Ephemeris source is now a session-global setting; new objects inherit
     *  the current global default from getDefaultQCConfig() at construction, so
     *  there is no per-row override to apply here (the import No-Ephemeris
     *  column was removed). */
}

const QCConfigurator::QCConfigRawFileInfo &QCConfigurator::getRawFileInfo() const
{
    return m_rawFileInfo;
}

void QCConfigurator::setQCConfig(const QCConfigData &config)
{
    m_config = config;
}

const QCConfigurator::QCConfigData &QCConfigurator::getQCConfig() const
{
    return m_config;
}

void QCConfigurator::changeInputFormat(RawFileDataFormatEnum format)
{
    m_rawFileInfo.rawFileDataFormat = format;
}

void QCConfigurator::setCheckConfig(const QCCheckConfigData &checkConfig)
{
    m_checkConfig = checkConfig;
}

const QCConfigurator::QCCheckConfigData &QCConfigurator::getCheckConfig() const
{
    return m_checkConfig;
}



int QCConfigurator::getAPIRawFileDataType(QCConfigurator::RawFileDataFormatEnum dataType)
{
    static const QMap<QCConfigurator::RawFileDataFormatEnum,int> s_dataTypeMap = {
        {QCConfigurator::RawFileDataFormatEnum::RTCM2,GNSS_STRFMT_RTCM2},
        {QCConfigurator::RawFileDataFormatEnum::RTCM3,GNSS_STRFMT_RTCM3},
        {QCConfigurator::RawFileDataFormatEnum::RINEX,GNSS_STRFMT_RINEX},
        {QCConfigurator::RawFileDataFormatEnum::HRCX,GNSS_STRFMT_HRCX},
        {QCConfigurator::RawFileDataFormatEnum::HRC,GNSS_STRFMT_HRC},
        {QCConfigurator::RawFileDataFormatEnum::RawMsgAscii,GNSS_STRFMT_RAWMSGA},
        {QCConfigurator::RawFileDataFormatEnum::RawMsgBinary,GNSS_STRFMT_RAWMSGB},
    };
    return s_dataTypeMap.value(dataType,GNSS_STRFMT_RINEX);
}

#ifdef CONSOLE

const CHCQC_GNSSCFG_T &QCConfigurator::getGNSSAPIConfig() const
{
    return m_gnssAPIConfig;
}

void QCConfigurator::setGNSSAPIConfig(const CHCQC_GNSSCFG_T &config, const QString &displayName)
{
    m_gnssAPIConfig = config;
    m_rawFileInfo.rawFilePath = displayName.isEmpty() ? QString::fromLocal8Bit(config.RoverFile) : displayName;  //for get QC object name
    /** Mirror the engine-config thresholds back into the grading config
     *  (inverse of generateAPIConfig()). updateQCStatus() compares engine
     *  statistics against m_checkConfig, which without this keeps the GUI
     *  defaults and grades every console run against thresholds the '-c'
     *  config never asked for. The engine keeps per-system arrays while the
     *  grading config keeps one per-freq-slot array, so each slot takes the
     *  value of the widest system covering it (GPS slots 0-4, BDS slot 5) and
     *  slots no engine system covers are zeroed (a zero threshold never
     *  fails). UseRate is a single engine value but per-system in the UI, so
     *  every system takes it (average round-trips exactly).
     *  XW1 (LMpMAX/LSnrMIN) is deliberately NOT mirrored: the per-freq-slot
     *  model cannot represent it losslessly and XW1 statistics flow into the
     *  QCSysComplex aggregate anyway, so console grading stays unchanged. */
    m_checkConfig.gpsUseRate = config.UseRate;
    m_checkConfig.bdsUseRate = config.UseRate;
    m_checkConfig.gloUseRate = config.UseRate;
    m_checkConfig.galUseRate = config.UseRate;
    m_checkConfig.qzssUseRate = config.UseRate;
    m_checkConfig.CycleJumpRatio = config.CJR;
    for (int i = 0; i < UI_COMPLEX_MP_MAX_NUM; ++i)
    {
        if (i < QC_GPS_FRE_NUM)
        {
            m_checkConfig.maxMP[i] = config.GMpMAX[i];
            m_checkConfig.minSNR[i] = config.GSnrMIN[i];
        }
        else if (i < QC_BDS_FRE_NUM)
        {
            m_checkConfig.maxMP[i] = config.CMpMAX[i];
            m_checkConfig.minSNR[i] = config.CSnrMIN[i];
        }
        else
        {
            m_checkConfig.maxMP[i] = 0;
            m_checkConfig.minSNR[i] = 0;
        }
    }
}

void QCConfigurator::setRinexConvertAPIConfig(const CHCQC_RINEXCFG_T &config, const QString &displayName)
{
    m_rinexAPIConfig = config;
    m_rawFileInfo.rawFilePath = displayName.isEmpty() ? QString::fromLocal8Bit(config.RawFile) : displayName;  //for get QC object name
}

const CHCQC_RINEXCFG_T &QCConfigurator::getRinexConvertAPIConfig() const
{
    return m_rinexAPIConfig;
}

#endif


CHCQC_GNSSCFG_T QCConfigurator::generateAPIConfig()
{
    CHCQC_GNSSCFG_T apiConfig;
    InitGNSSCfg(&apiConfig);
    //config
    apiConfig.Mode = -1;
    apiConfig.RawFileDataType = getAPIRawFileDataType(m_rawFileInfo.rawFileDataFormat);
    apiConfig.Sys = 0;
    if(m_config.sysGPSEnable)
    {
        apiConfig.Sys |= QC_SYS_GPS;
    }
    if(m_config.sysBDSEnable)
    {
        apiConfig.Sys |= QC_SYS_CMP;
    }
    if(m_config.sysGLOEnable)
    {
        apiConfig.Sys |= QC_SYS_GLO;
    }
    if(m_config.sysGALEnable)
    {
        apiConfig.Sys |= QC_SYS_GAL;
    }
    if(m_config.sysQZSSEnable)
    {
        apiConfig.Sys |= QC_SYS_QZS;
    }
    if(m_config.sysXW1Enable)
    {
        apiConfig.Sys |= QC_SYS_XW1;
    }

    /**< All-unchecked fallback = the full six-system mask including XW1, aligned
     *  with the engine (an empty Sys string maps to 255 = all systems incl.
     *  0x80). Sibling surfaces for reference: the config file editor instead
     *  rejects an empty selection outright (QCConfigFileModel "At least one
     *  GNSS system"), so only this dialog path needs the fallback. */
    if(0 == apiConfig.Sys)
    {
        apiConfig.Sys = QC_SYS_GPS + QC_SYS_CMP + QC_SYS_GLO + QC_SYS_GAL + QC_SYS_QZS + QC_SYS_XW1;
    }

    apiConfig.AllFreq = 1;
    /**< Map the three-way ephemeris enum to the engine's EphMode field and,
     *  in custom-file mode, inject the user-specified nav path. Encoding follows
     *  the same PathUtils pattern as RoverFile below so non-ASCII paths survive
     *  the C layer's ANSI-codepage fopen(). */
    switch (m_config.ephemerisMode)
    {
        case QCConfigData::EphemerisModeEnum::EphemerisNone:
            apiConfig.EphMode = QC_EPH_MODE_NONE;
            break;
        case QCConfigData::EphemerisModeEnum::EphemerisCustomFile:
            apiConfig.EphMode = QC_EPH_MODE_SPECIFIED;
            if (!m_config.ephemerisFilePaths.isEmpty())
            {
                PathUtils::copyFilePathBytes(apiConfig.RinexEphFile, m_config.ephemerisFilePaths);
            }
            break;
        case QCConfigData::EphemerisModeEnum::EphemerisDefault:
        default:
            apiConfig.EphMode = QC_EPH_MODE_AUTO;
            break;
    }

//    this->parseSysFreqOption({"L1", "L2C", "L5", "L1C-P", "L2P"},QCResultManager::getSystemIndex(QCResultManager::QCSysGPS),&apiConfig);
//    this->parseSysFreqOption({"B1","B2","B3","B1C","B2a","B2b"},QCResultManager::getSystemIndex(QCResultManager::QCSysBDS),&apiConfig);
//    this->parseSysFreqOption({"G1","G2"},QCResultManager::getSystemIndex(QCResultManager::QCSysGLO),&apiConfig);
//    this->parseSysFreqOption({"E1","E5A","E5B","E5(A+B)","E6"},QCResultManager::getSystemIndex(QCResultManager::QCSysGAL),&apiConfig);
//    this->parseSysFreqOption({"L1","L2","L5","L5S","L1C(P)","L1S"},QCResultManager::getSystemIndex(QCResultManager::QCSysQZSS),&apiConfig);

    /** Use named QByteArray to avoid double-temporary UB (same pattern as RoverFile). */
    QByteArray banPrnBytes = m_config.banPrn.toLocal8Bit();
    memcpy(apiConfig.BanPRN, banPrnBytes.constData(),
           qMin(banPrnBytes.size(), QC_MAX_NUM_SAT - 1));
    apiConfig.EleDeg = m_config.eleDeg;
    apiConfig.Interval = m_config.interval;
    for (int i = 0; i < m_config.snrGroup.size() && i < QC_MAXELEVINTERV; ++i)
    {
        apiConfig.SNRGroup[i] = m_config.snrGroup[i];
    }
    apiConfig.SNRGroupSize = qMin<int>(m_config.snrGroup.size(),QC_MAXELEVINTERV);
    if(m_config.startTime.isValid())
    {
        apiConfig.StartTime = dateTimeToQCTime(m_config.startTime);
    }
    if(m_config.endTime.isValid())
    {
        apiConfig.EndTime = dateTimeToQCTime(m_config.endTime);
    }
    if(m_rawFileInfo.rtcmTime.isValid())
    {
        apiConfig.RTCMTime = dateTimeToQCTime(m_rawFileInfo.rtcmTime);
    }
    /** Encode via PathUtils so non-ASCII paths survive the C layer's
     *  ANSI-codepage fopen() (Windows: 8.3 short path + toLocal8Bit fallback). */
    QByteArray roverPath = PathUtils::toLocalFilePathBytes(m_rawFileInfo.rawFilePath);
    memcpy(apiConfig.RoverFile, roverPath.constData(),
           qMin(roverPath.size(), (int)sizeof(apiConfig.RoverFile) - 1));
    QString outDir = this->getWorkDirPath();
    /** toLocalDirPathBytes normalizes the trailing separator, creates the
     *  directory, and encodes the path for the C layer's fopen(). */
    QByteArray outDirBytes = PathUtils::toLocalDirPathBytes(outDir);
    memcpy(apiConfig.OutDir, outDirBytes.constData(), outDirBytes.size());
    //check config
    apiConfig.UseRate = m_checkConfig.getTotalUseRate();
    apiConfig.CJR = m_checkConfig.CycleJumpRatio;
    for (int i = 0; i < UI_COMPLEX_MP_MAX_NUM; ++i)
    {
        if(i < QC_GPS_FRE_NUM)
        {
            apiConfig.GMpMAX[i] = m_checkConfig.maxMP[i];
            apiConfig.GSnrMIN[i] = m_checkConfig.minSNR[i];
        }
        if(i < QC_GLONASS_FRE_NUM)
        {
            apiConfig.RMpMAX[i] = m_checkConfig.maxMP[i];
            apiConfig.RSnrMIN[i] = m_checkConfig.minSNR[i];
        }
        if(i < QC_GALILEO_FRE_NUM)
        {
            apiConfig.EMpMAX[i] = m_checkConfig.maxMP[i];
            apiConfig.ESnrMIN[i] = m_checkConfig.minSNR[i];
        }
        if(i < QC_BDS_FRE_NUM)
        {
            apiConfig.CMpMAX[i] = m_checkConfig.maxMP[i];
            apiConfig.CSnrMIN[i] = m_checkConfig.minSNR[i];
        }
        if(i < QC_QZSS_FRE_NUM)
        {
            apiConfig.JMpMAX[i] = m_checkConfig.maxMP[i];
            apiConfig.JSnrMIN[i] = m_checkConfig.minSNR[i];
        }
        if(i < QC_XW1_FRE_NUM)
        {
            apiConfig.LMpMAX[i] = m_checkConfig.maxMP[i];
            apiConfig.LSnrMIN[i] = m_checkConfig.minSNR[i];
        }
    }

    return apiConfig;
}

CHCQC_RINEXCFG_T QCConfigurator::generateAPIRinexConvertConfig(const RinexConvertSettings &settings) const
{
    CHCQC_RINEXCFG_T rinexConfig = {};
    InitRinexCfg(&rinexConfig);
    rinexConfig.DataType = getAPIRawFileDataType(m_rawFileInfo.rawFileDataFormat);
    rinexConfig.OutputType = DATA_OUT_TYPE_RINEX; // Rinex
    rinexConfig.RinexVer = settings.rinexVersion;
    rinexConfig.Interval = settings.interval;
    if(m_rawFileInfo.rtcmTime.isValid())
    {
        rinexConfig.RTCMTime = dateTimeToQCTime(m_rawFileInfo.rtcmTime);
    }
    rinexConfig.AllFreq = TRUE;
    QStringList systemList;
    if(settings.sysGPSEnable)
    {
        systemList.append("G");
    }
    if(settings.sysGLOEnable)
    {
        systemList.append("R");
    }
    if(settings.sysGALEnable)
    {
        systemList.append("E");
    }
    if(settings.sysQZSSEnable)
    {
        systemList.append("J");
    }
    if(settings.sysBDSEnable)
    {
        systemList.append("C");
    }
    QByteArray systems = systemList.join(",").toLocal8Bit();
    memcpy(rinexConfig.Sys,systems.data(),systems.size());
    /**< Splitting is no longer supported by the product: Split stays 0 (no
     *  split, from the zero-init above plus InitRinexCfg) and SplitStart/
     *  SplitEnd are never set. */
    /** Encode via PathUtils (same rationale as generateAPIConfig). */
    QByteArray rawPath = PathUtils::toLocalFilePathBytes(m_rawFileInfo.rawFilePath);
    memcpy(rinexConfig.RawFile, rawPath.constData(),
           qMin(rawPath.size(), (int)sizeof(rinexConfig.RawFile) - 1));
    QString outDir = this->getWorkDirPath();
    /** toLocalDirPathBytes normalizes the trailing separator, creates the
     *  directory, and encodes the path for the C layer's fopen(). */
    QByteArray outDirBytes = PathUtils::toLocalDirPathBytes(outDir);
    memcpy(rinexConfig.OutDir, outDirBytes.constData(), outDirBytes.size());

    return rinexConfig;
}

QString QCConfigurator::getFileDataFormatName(RawFileDataFormatEnum format)
{
    switch (format)
    {
    case RawFileDataFormatEnum::RTCM2:
        return "RTCM2";
    case RawFileDataFormatEnum::RTCM3:
        return "RTCM3";
    case RawFileDataFormatEnum::RINEX:
        return "RINEX";
    case RawFileDataFormatEnum::HRCX:
        return "HRCX";
    case RawFileDataFormatEnum::HRC:
        return "HRC";
    case RawFileDataFormatEnum::RawMsgAscii:
        return "RawMsgAscii";
    case RawFileDataFormatEnum::RawMsgBinary:
        return "RawMsgBinary";
    default:
        break;
    }
    return QString(QObject::tr("Please select"));
}

void QCConfigurator::addFileDataFormatItems(QComboBox *comboBox, bool includeNone)
{
    if (includeNone)
    {
        comboBox->addItem(getFileDataFormatName(RawFileDataFormatEnum::None), RawFileDataFormatEnum::None);
    }
    const QVector<RawFileDataFormatEnum> selectableFormats = {
        RawFileDataFormatEnum::RTCM2,
        RawFileDataFormatEnum::RTCM3,
        RawFileDataFormatEnum::RINEX,
        RawFileDataFormatEnum::HRCX,
        RawFileDataFormatEnum::HRC,
        RawFileDataFormatEnum::RawMsgAscii,
        RawFileDataFormatEnum::RawMsgBinary,
    };
    for (const RawFileDataFormatEnum format : selectableFormats)
    {
        comboBox->addItem(getFileDataFormatName(format), format);
    }
}

QCConfigurator::RawFileDataFormatEnum QCConfigurator::getFormatFromSuffix(const QString &suffix)
{
    const QString lowerSuffix = suffix.toLower();

    // RINEX observation format (zero or more digits followed by 'o', e.g.
    // "o", "25o"); ephemeris suffixes (p/n/c/g/l) are deliberately not
    // guessed - the import flow skips them before guessing and the batch
    // whitelist does not accept them either.
    static QRegularExpression rinexObsRegex(R"(^\d*o$)");
    if (rinexObsRegex.match(lowerSuffix).hasMatch())
    {
        return QCConfigurator::RawFileDataFormatEnum::RINEX;
    }

    if (lowerSuffix == "rtcm3")
    {
        return QCConfigurator::RawFileDataFormatEnum::RTCM3;
    }
    if (lowerSuffix == "rtcm2")
    {
        return QCConfigurator::RawFileDataFormatEnum::RTCM2;
    }
    if (lowerSuffix == "hrc")
    {
        return QCConfigurator::RawFileDataFormatEnum::HRC;
    }

    return QCConfigurator::RawFileDataFormatEnum::None;
}

bool QCConfigurator::isBatchCandidateSuffix(const QString &suffix)
{
    const QString lowerSuffix = suffix.toLower();
    if(getFormatFromSuffix(suffix) != QCConfigurator::RawFileDataFormatEnum::None)
    {
        return true;
    }
    // Suffixes with no unambiguous format mapping but a stable naming
    // convention: left to the engine (HCN header / -1 auto detection).
    static const QStringList fixedSuffixes = { "hrcx", "hcn" };
    return fixedSuffixes.contains(lowerSuffix);
}

QCConfigurator::QCConfigData QCConfigurator::initConfig()
{
    QCConfigurator::QCConfigData config;
    config.interval = 0;
    config.eleDeg = 10;
    config.sysGPSEnable = true;
    config.sysBDSEnable = true;
    config.sysGLOEnable = true;
    config.sysGALEnable = true;
    config.sysQZSSEnable = true;
    config.sysXW1Enable = true;
    config.snrGroup = {30,90};
    config.ephemerisMode = QCConfigData::EphemerisModeEnum::EphemerisDefault;
    /**< ephemerisFilePaths is default-constructed empty; no .clear() needed. */

    return config;
}

QString QCConfigurator::getReportHtmlFilePath() const
{
    return QString("%1_QC.html").arg(this->getWorkDirPath().append("/").append(QFileInfo(m_rawFileInfo.rawFilePath).baseName()));
}

QString QCConfigurator::getDiagnoseHtmlFilePath() const
{
    // Use completeBaseName() to match the naming written by
    // QCDiagnoseReportExporter when it builds per-object output paths.
    return QString("%1_QCDiag.html").arg(this->getWorkDirPath().append("/").append(QFileInfo(m_rawFileInfo.rawFilePath).completeBaseName()));
}

QString QCConfigurator::getWorkDirPath() const
{
    if(WorkDirModeEnum::SameAsSourceFileDir == m_config.workDirMode)
    {
        return QFileInfo(m_rawFileInfo.rawFilePath).absolutePath();
    }
    return m_config.otherWorkDirPath;
}
