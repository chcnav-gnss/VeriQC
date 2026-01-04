/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI QC Object Module
 * *-
 * @file QCConfigurator.h
 * @author CHC
 * @date 2025-09-28
 * @brief management QC Object config
 * 
**************************************************************************/
#ifndef QCCONFIGURATOR_H
#define QCCONFIGURATOR_H

#include <QDateTime>
#include <QObject>
#include <QVector>
#include <QStringList>
#include "VeriQC.h"
#include "QCResultManager.h"


class QCConfigurator
{
public:
    enum RawFileDataFormatEnum
    {
        None,
        RTCM2,
        RTCM3,
        RINEX,
        HRCX,
        RawMsgAscii,
        RawMsgBinary,
    };

    enum WorkDirModeEnum
    {
        SameAsSourceFileDir,
        OtherDir
    };

    struct QCConfigRawFileInfo
    {
        RawFileDataFormatEnum rawFileDataFormat = RawFileDataFormatEnum::None;
        QDateTime rtcmTime;
        QString rawFilePath;
    };

    struct QCConfigData
    {
        float interval = 0;
        unsigned int eleDeg = 10;
        bool sysGPSEnable = true;
        bool sysBDSEnable = true;
        bool sysGLOEnable = true;
        bool sysGALEnable = true;
        bool sysQZSSEnable = true;
        QDateTime startTime;
        QDateTime endTime;
        QString banPrn;
        QVector<int> snrGroup;
        WorkDirModeEnum workDirMode = WorkDirModeEnum::SameAsSourceFileDir;
        QString otherWorkDirPath;
    };

    struct QCCheckConfigData
    {
        float gpsUseRate = 0;
        float bdsUseRate = 0;
        float gloUseRate = 0;
        float galUseRate = 0;
        float qzssUseRate = 0;
        unsigned int CycleJumpRatio = 0;
        float maxMP[UI_COMPLEX_MP_MAX_NUM] = {0};
        float minSNR[UI_COMPLEX_MP_MAX_NUM] = {0};

        inline float getTotalUseRate()const{
            return (gpsUseRate + bdsUseRate + gloUseRate + galUseRate + qzssUseRate)/5;
        }
    };

    struct RinexConvertSettings
    {
        unsigned int rinexVersion = 304;  // e.g. 304 : 3.04
        unsigned int interval = 0;
        bool timePeriodSplit = false;
        QDateTime startSplitTime;
        QDateTime endSplitTime;
        bool sysGPSEnable = true;
        bool sysBDSEnable = true;
        bool sysGLOEnable = true;
        bool sysGALEnable = true;
        bool sysQZSSEnable = true;
    };
public:
    QCConfigurator();
    QString getFileName()const;
    void setRawFileInfo(const QCConfigRawFileInfo & info);
    const QCConfigRawFileInfo &getRawFileInfo()const;
    void setQCConfig(const QCConfigData &config);
    const QCConfigData &getQCConfig()const;
    void changeInputFormat(RawFileDataFormatEnum format);
    void setCheckConfig(const QCCheckConfigData &checkConfig);
    const QCCheckConfigData &getCheckConfig()const;
    CHCQC_GNSSCFG_T generateAPIConfig();
    CHCQC_RINEXCFG_T generateAPIRinexConvertConfig(const RinexConvertSettings &settings)const;
    static QString getFileDataFormatName(RawFileDataFormatEnum format);
    static QCConfigData initConfig();
    QString getReportHtmlFilePath()const;
    QString getWorkDirPath()const;
    static int getAPIRawFileDataType(QCConfigurator::RawFileDataFormatEnum dataType);
#ifdef CONSOLE
    const CHCQC_GNSSCFG_T &getGNSSAPIConfig()const;
    void setGNSSAPIConfig(const CHCQC_GNSSCFG_T &config);
    void setRinexConvertAPIConfig(const CHCQC_RINEXCFG_T &config);
    const CHCQC_RINEXCFG_T &getRinexConvertAPIConfig()const;
#endif

private:
    QCConfigRawFileInfo m_rawFileInfo;
    QCConfigData m_config;
    QCCheckConfigData m_checkConfig;
#ifdef CONSOLE
    CHCQC_GNSSCFG_T m_gnssAPIConfig;
    CHCQC_RINEXCFG_T m_rinexAPIConfig;
#endif
};
Q_DECLARE_METATYPE(QCConfigurator::RawFileDataFormatEnum)

#endif // QCCONFIGURATOR_H
