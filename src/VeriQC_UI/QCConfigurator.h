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

class QComboBox;

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
        HRC,
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
        /**< Ephemeris source selection (mutually exclusive). Replaces the legacy
         *   bool noEphEnable toggle with a three-way choice. */
        enum EphemerisModeEnum
        {
            EphemerisDefault,   /**< use data's own ephemeris (engine auto-discovers nav by OBS suffix) */
            EphemerisNone,      /**< run QC without ephemeris (legacy no-ephemeris mode) */
            EphemerisCustomFile /**< use a user-specified RINEX nav file */
        };

        float interval = 0;
        unsigned int eleDeg = 10;
        bool sysGPSEnable = true;
        bool sysBDSEnable = true;
        bool sysGLOEnable = true;
        bool sysGALEnable = true;
        bool sysQZSSEnable = true;
        bool sysXW1Enable = true;
        QDateTime startTime;
        QDateTime endTime;
        QString banPrn;
        QVector<int> snrGroup;
        WorkDirModeEnum workDirMode = WorkDirModeEnum::SameAsSourceFileDir;
        QString otherWorkDirPath;
        EphemerisModeEnum ephemerisMode = EphemerisModeEnum::EphemerisDefault;
        /**< Single RINEX nav file path, valid only when ephemerisMode == EphemerisCustomFile */
        QString ephemerisFilePaths;

        /**< Derived legacy flag: true only in no-ephemeris mode. Preserves the
         *   semantics of the old bool noEphEnable for downstream consumers
         *   (sky plot disable, Excel N/A, diagnosis input). */
        inline bool noEphEnable() const { return ephemerisMode == EphemerisModeEnum::EphemerisNone; }

        /**< Field-by-field equality, used to detect an actual config change
         *   when a config dialog is saved (the re-QC prompt must stay silent
         *   on untouched values). Compare every stored field and keep this
         *   list in sync when adding new ones. Floats round-trip through the
         *   dialog's controls, so a plain == is exact enough here. */
        bool operator==(const QCConfigData &other) const
        {
            return interval == other.interval
                && eleDeg == other.eleDeg
                && sysGPSEnable == other.sysGPSEnable
                && sysBDSEnable == other.sysBDSEnable
                && sysGLOEnable == other.sysGLOEnable
                && sysGALEnable == other.sysGALEnable
                && sysQZSSEnable == other.sysQZSSEnable
                && sysXW1Enable == other.sysXW1Enable
                && startTime == other.startTime
                && endTime == other.endTime
                && banPrn == other.banPrn
                && snrGroup == other.snrGroup
                && workDirMode == other.workDirMode
                && otherWorkDirPath == other.otherWorkDirPath
                && ephemerisMode == other.ephemerisMode
                && ephemerisFilePaths == other.ephemerisFilePaths;
        }
        inline bool operator!=(const QCConfigData &other) const { return !(*this == other); }   /**< inverse of operator== */
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

        /**< Field-by-field equality, counterpart of QCConfigData::operator==:
         *   detects an actual threshold change when a config dialog is saved. */
        bool operator==(const QCCheckConfigData &other) const
        {
            if (gpsUseRate != other.gpsUseRate || bdsUseRate != other.bdsUseRate
                || gloUseRate != other.gloUseRate || galUseRate != other.galUseRate
                || qzssUseRate != other.qzssUseRate
                || CycleJumpRatio != other.CycleJumpRatio)
            {
                return false;
            }
            for (int i = 0; i < UI_COMPLEX_MP_MAX_NUM; ++i)
            {
                if (maxMP[i] != other.maxMP[i] || minSNR[i] != other.minSNR[i])
                {
                    return false;
                }
            }
            return true;
        }
        inline bool operator!=(const QCCheckConfigData &other) const { return !(*this == other); }   /**< inverse of operator== */
    };

    struct RinexConvertSettings
    {
        unsigned int rinexVersion = 304;  // e.g. 304 : 3.04
        float interval = 0;
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
    /**
     * @brief Populates a combo box with the raw file data formats selectable in the UI.
     *
     * Single source of truth for the format dropdown item lists (import dialog
     * batch-apply combo box, per-file format combo boxes and the station
     * attribute widget), so adding a selectable format only requires updating
     * this function instead of every call site.
     *
     * @param comboBox Combo box to append the format items to.
     * @param includeNone Also prepend the None ("Please select") entry.
     */
    static void addFileDataFormatItems(QComboBox *comboBox, bool includeNone = false);

    /**
     * @brief Guess the raw data format from a file suffix (lower-cased).
     *
     * Shared single source for suffix-based format guessing: RINEX observation
     * suffixes (zero or more digits followed by 'o', e.g. "o", "25o", "125o";
     * ephemeris suffixes p/n/c/g/l are intentionally not guessed), rtcm3,
     * rtcm2 and hrc. Returns None when the suffix is not recognized.
     */
    static RawFileDataFormatEnum getFormatFromSuffix(const QString &suffix);

    /**
     * @brief Whether a file suffix qualifies a file for command-line batch
     *        processing (whitelist check, case-insensitive).
     *
     * Accepts the RINEX observation pattern used by getFormatFromSuffix() plus
     * the fixed suffixes rtcm2/rtcm3/hrc/hrcx/hcn. Formats without a stable
     * suffix (RawMsg ASCII/Binary, and HCN files not named *.hcn) are not
     * covered.
     */
    static bool isBatchCandidateSuffix(const QString &suffix);
    static QCConfigData initConfig();
    QString getReportHtmlFilePath()const;
    /**
     * @brief Default output path of the QC diagnosis HTML report.
     *
     * Mirrors getReportHtmlFilePath() but follows the <basename>_QCDiag.html
     * naming convention established by QCDiagnoseReportExporter. Uses
     * completeBaseName() so multi-suffix raw file names produce the same
     * stem the exporter already writes in batch mode.
     *
     * @return Absolute file path under getWorkDirPath().
     */
    QString getDiagnoseHtmlFilePath()const;
    QString getWorkDirPath()const;
    static int getAPIRawFileDataType(QCConfigurator::RawFileDataFormatEnum dataType);

    /**< Whether this object's ephemeris config was explicitly customized via
     *   the per-object config dialog. When true, a global-config save skips
     *   this object so its ephemeris override is preserved (session lifetime). */
    inline bool isEphemerisModeCustomized() const { return m_ephemerisModeCustomized; }
    inline void setEphemerisModeCustomized(bool customized) { m_ephemerisModeCustomized = customized; }
#ifdef CONSOLE
    const CHCQC_GNSSCFG_T &getGNSSAPIConfig()const;
    /**
     * @brief Store a QC-mode engine config for this object (console batch path).
     * @param config Fully prepared CHCQC_GNSSCFG_T; RoverFile/OutDir must
     *               already be encoded via PathUtils.
     * @param displayName Original (pre-encoding) file path used for object
     *                    naming; the char[] field itself may hold an 8.3 short
     *                    path on Chinese-locale Windows, which is unreadable
     *                    in progress/Excel.
     */
    void setGNSSAPIConfig(const CHCQC_GNSSCFG_T &config, const QString &displayName = QString());
    /**
     * @brief Store a Rinex-convert-mode engine config for this object (console batch path).
     * @param config Fully prepared CHCQC_RINEXCFG_T; RawFile/OutDir must
     *               already be encoded via PathUtils.
     * @param displayName Original (pre-encoding) file path used for object
     *                    naming; the char[] field itself may hold an 8.3 short
     *                    path on Chinese-locale Windows, which is unreadable
     *                    in progress/Excel.
     */
    void setRinexConvertAPIConfig(const CHCQC_RINEXCFG_T &config, const QString &displayName = QString());
    const CHCQC_RINEXCFG_T &getRinexConvertAPIConfig()const;
#endif

private:
    QCConfigRawFileInfo m_rawFileInfo;
    QCConfigData m_config;
    QCCheckConfigData m_checkConfig;
    bool m_ephemerisModeCustomized = false;
#ifdef CONSOLE
    CHCQC_GNSSCFG_T m_gnssAPIConfig;
    CHCQC_RINEXCFG_T m_rinexAPIConfig;
#endif
};
Q_DECLARE_METATYPE(QCConfigurator::RawFileDataFormatEnum)

#endif // QCCONFIGURATOR_H
