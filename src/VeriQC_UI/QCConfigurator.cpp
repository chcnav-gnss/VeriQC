#include "QCConfigurator.h"
#include <QMap>
#include <QFileInfo>
#include <QDir>
#include "QCResultManager.h"
#include "AppSettingsManagement.h"

#define GNSS_STRFMT_RTCM2			0		/**< stream format: RTCM 2 */
#define GNSS_STRFMT_RTCM3			1		/**< stream format: RTCM 3 */
#define GNSS_STRFMT_RINEX			11		/**< stream format: RINEX */
#define GNSS_STRFMT_HRCX			37		/**< stream format: HRCX */
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

void QCConfigurator::setGNSSAPIConfig(const CHCQC_GNSSCFG_T &config)
{
    m_gnssAPIConfig = config;
    m_rawFileInfo.rawFilePath = config.RoverFile;  //for get QC object name
}

void QCConfigurator::setRinexConvertAPIConfig(const CHCQC_RINEXCFG_T &config)
{
    m_rinexAPIConfig = config;
    m_rawFileInfo.rawFilePath = config.RawFile;  //for get QC object name
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
    if(m_config.sysGALEnable)
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

    if(0 == apiConfig.Sys)
    {
        apiConfig.Sys = QC_SYS_GPS + QC_SYS_CMP + QC_SYS_GLO + QC_SYS_GAL + QC_SYS_QZS;
    }

    apiConfig.AllFreq = 1;

//    this->parseSysFreqOption({"L1", "L2C", "L5", "L1C-P", "L2P"},QCResultManager::getSystemIndex(QCResultManager::QCSysGPS),&apiConfig);
//    this->parseSysFreqOption({"B1","B2","B3","B1C","B2a","B2b"},QCResultManager::getSystemIndex(QCResultManager::QCSysBDS),&apiConfig);
//    this->parseSysFreqOption({"G1","G2"},QCResultManager::getSystemIndex(QCResultManager::QCSysGLO),&apiConfig);
//    this->parseSysFreqOption({"E1","E5A","E5B","E5(A+B)","E6"},QCResultManager::getSystemIndex(QCResultManager::QCSysGAL),&apiConfig);
//    this->parseSysFreqOption({"L1","L2","L5","L5S","L1C(P)","L1S"},QCResultManager::getSystemIndex(QCResultManager::QCSysQZSS),&apiConfig);

    memcpy(apiConfig.BanPRN,m_config.banPrn.toLocal8Bit().data(),qMin(m_config.banPrn.toLocal8Bit().size(),QC_MAX_NUM_SAT-1));
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
    memcpy(apiConfig.RoverFile,m_rawFileInfo.rawFilePath.toLocal8Bit().data(),m_rawFileInfo.rawFilePath.toLocal8Bit().size());
    QString outDir = this->getWorkDirPath();
    outDir += QDir::separator();
    memcpy(apiConfig.OutDir,outDir.toLocal8Bit().data(),outDir.toLocal8Bit().size());
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
    rinexConfig.Split = settings.timePeriodSplit ? 3 : 0;
    if(settings.timePeriodSplit)
    {
        rinexConfig.SplitStart = dateTimeToQCTime(settings.startSplitTime);
        rinexConfig.SplitEnd = dateTimeToQCTime(settings.endSplitTime);
    }
    memcpy(rinexConfig.RawFile,m_rawFileInfo.rawFilePath.toLocal8Bit().data(),m_rawFileInfo.rawFilePath.toLocal8Bit().size());
    QString outDir = this->getWorkDirPath();
    outDir += QDir::separator();
    memcpy(rinexConfig.OutDir,outDir.toLocal8Bit().data(),outDir.toLocal8Bit().size());

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
    case RawFileDataFormatEnum::RawMsgAscii:
        return "RawMsgAscii";
    case RawFileDataFormatEnum::RawMsgBinary:
        return "RawMsgBinary";
    default:
        break;
    }
    return QString(QObject::tr("Please select"));
}

QCConfigurator::QCConfigData QCConfigurator::initConfig()
{
    QCConfigurator::QCConfigData config;
    config.interval = 1.0;
    config.eleDeg = 10;
    config.sysGPSEnable = true;
    config.sysBDSEnable = true;
    config.sysGLOEnable = true;
    config.sysGALEnable = true;
    config.sysQZSSEnable = true;
    config.snrGroup = {30};

    return config;
}

QString QCConfigurator::getReportHtmlFilePath() const
{
    return QString("%1_QC.html").arg(this->getWorkDirPath().append("/").append(QFileInfo(m_rawFileInfo.rawFilePath).baseName()));
}

QString QCConfigurator::getWorkDirPath() const
{
    if(WorkDirModeEnum::SameAsSourceFileDir == m_config.workDirMode)
    {
        return QFileInfo(m_rawFileInfo.rawFilePath).absolutePath();
    }
    return m_config.otherWorkDirPath;
}
