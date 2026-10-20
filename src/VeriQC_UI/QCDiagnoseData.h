/**********************************************************************//**
 * 		VeriQC
 *
 * 		UI Diagnostic Data Module
 * *-
 * @file QCDiagnoseData.h
 * @author CHC
 * @date 2026-06-15
 * @brief Data structures and derivation helpers consumed by the
 *        diagnostic engine.
 *
 * The pipeline is:
 *   QC_RESULT_T --buildDiagnosticInput()--> DiagnosticInput
 *   DiagnosticInput --DiagnoseEngine::diagnose()--> DiagnosticResult
 *   DiagnosticResult --QCDiagnoseTextBuilder--> localized paragraphs
 *
 * Negative-ratio sentinel (-1.0) emitted by the lower layer when the
 * denominator is zero is filtered inside buildDiagnosticInput, so the
 * diagnostic engine can rely on non-negative metrics (sysOSR uses the
 * 99999.0 sentinel to denote "no slip observed").
 *
 **************************************************************************/
#ifndef QCDIAGNOSEDATA_H
#define QCDIAGNOSEDATA_H

#include <QDateTime>
#include <QList>
#include <QString>
#include "VeriQC.h"
#include "QCResultManager.h"

/**
 * @namespace QCDiag
 * @brief All types and functions that belong to the diagnostic-summary
 *        subsystem. Constants live in the sibling QCDiagConst namespace.
 */
namespace QCDiag
{
/**
 * @brief Cycle-slip derived metrics.
 *
 * Ratios are fractions in [0, 1]. Durations are seconds.
 */
struct SlipMetrics
{
    double totalDurationSec = 0.0;       /**< Observation span of the file. */
    double avgHealthyArcDurationSec = 0.0; /**< Raw copy of AvgHealthyArcDurationSec. */
    double maxHealthyArcDurationSec = 0.0; /**< Raw copy of MaxHealthyArcDurationSec. */
    double avgHealthyArcRatio = 0.0;     /**< avgHealthyArcDurationSec / totalDurationSec. */
    double maxHealthyArcRatio = 0.0;     /**< maxHealthyArcDurationSec / totalDurationSec. */
    double slipEpochRatioOverall = 0.0;  /**< sum(OutLimitEpochNum)/sum(WindowEpochNum). */
    double avgOSR = 0.0;                 /**< Overall slip ratio; 99999.0 means "no slip". */
    double sysOSR[QC_MAX_NUM_SYS] = {0}; /**< OSR per system; 99999.0 means "no slip". */

    /**
     * @brief Aggregated view of one slip-outlier time window.
     */
    struct SlipWindowSummary
    {
        QDateTime startTime;             /**< WindowStartTime converted to Qt datetime. */
        unsigned int epochNum = 0;       /**< WindowEpochNum. */
        unsigned int outLimitEpochNum = 0; /**< OutLimitEpochNum. */
        double outLimitEpochRatio = 0.0; /**< OutLimitEpochRatio. */
        int worstSatIndex = -1;          /**< Satellite index with max SatSlipRatio; -1 if none. */
        double worstSatSlipRatio = 0.0;  /**< Slip ratio of worstSatIndex. */
        bool hasSevereSat = false;       /**< True if worstSatSlipRatio >= kSlipSatRatioAlert. */
    };
    QList<SlipWindowSummary> outlierWindows; /**< Length = valid SlipOutlierWindowCount. */
};

/**
 * @brief SNR derived metrics.
 */
struct SnrMetrics
{
    double avgSNR[QC_MAX_NUM_SYS][QC_NUM_FREQ] = {{0}}; /**< Direct copy of SNR[file][sys][freq]. */
    double lowCN0SatRatio = 0.0;        /**< LowCN0SatRatio; -1.0 filtered to 0.0. */
    double lowCN0EpochRatio = 0.0;      /**< LowCN0EpochRatio; -1.0 filtered to 0.0. */
    int worstFreqIndex = -1;            /**< Freq index > 10 dB below peer mean; -1 if none. */
    double worstFreqAvgDropDb = 0.0;    /**< Magnitude of the drop for worstFreqIndex. */

    /**
     * @brief Aggregated view of one CN0 drop time window.
     */
    struct CN0DropWindowSummary
    {
        QDateTime startTime;            /**< WindowStartTime. */
        float startAvgCN0 = 0.0f;       /**< WindowStartAvgCN0. */
        float avgCN0Drop = 0.0f;        /**< AvgCN0Drop. */
        bool isAlert = false;           /**< True when avgCN0Drop > kSnrWindowDropDb. */
    };
    QList<CN0DropWindowSummary> dropWindows; /**< Length = valid CN0DropWindowCount. */
    int alertDropWindowCount = 0;       /**< Number of windows with isAlert == true. */
};

/**
 * @brief Multipath derived metrics.
 *
 * groupRatio/groupRatioAve retain the -1.0 sentinel for "no observations";
 * engine code must skip negative entries when aggregating.
 */
struct MpMetrics
{
    /** [sys][freq][group] copy of MPGroupPercent[file][sys][freq][group]. */
    float groupRatio[QC_MAX_NUM_SYS][QC_NUM_FREQ][MP_GROUP_PERCENT_SIZE] = {{{0}}};
    /** [freq][group] copy of MPGroupPercentAve[file][freq][group]. */
    float groupRatioAve[QC_NUM_FREQ][MP_GROUP_PERCENT_SIZE] = {{0}};
    int worstSysIndex = -1;             /**< System with lowest <0.3m ratio; -1 if none or >=40%. */
    int worstFreqIndex = -1;            /**< Frequency with lowest valid <0.3m ratio; -1 if none. */
};

/**
 * @brief Availability / observation-break derived metrics.
 */
struct AvailabilityMetrics
{
    double useRateTotal = 0.0;          /**< Overall use rate. */
    double useRateSys[QC_MAX_NUM_SYS] = {0}; /**< Per-system use rate. */
    int sysCount = 0;                   /**< Number of systems with positive use rate. */
    int outlierSysIndex = -1;           /**< System with use rate 20%+ below peers; -1 if none. */
    double outlierSysUseRate = 0.0;     /**< Use rate of the outlier system. */
    double peerAvgUseRate = 0.0;        /**< Mean use rate of the remaining systems. */

    int obsBreakCount = 0;              /**< Total observation break count. */

    /** Systems flagged as "observed but use rate is zero" (FR-09 anomaly).
     *  Set true when getValidSysSatPrns returns a non-empty PRN list for the
     *  system yet UseRateSys == 0. Drives the availability-section remark. */
    bool sysObserved[QC_MAX_NUM_SYS] = {false};

    /**
     * @brief Aggregated view of one observation break.
     */
    struct BreakSummary
    {
        QDateTime startTime;            /**< BreakStartTime. */
        float durationSec = 0.0f;       /**< BreakDurationSec. */
    };
    QList<BreakSummary> topBreaks;      /**< Length = min(ObsBreakCount, QC_DIAG_TOP_BREAK_NUM). */
};

/**
 * @brief Aggregated diagnostic input for a single QC object.
 */
struct DiagnosticInput
{
    QString fileName;                   /**< Source file base name. */
    QDateTime startTime;                /**< Observation start. */
    QDateTime endTime;                  /**< Observation end. */
    SlipMetrics slip;                   /**< Cycle-slip derived metrics. */
    SnrMetrics snr;                     /**< SNR derived metrics. */
    MpMetrics mp;                       /**< Multipath derived metrics. */
    AvailabilityMetrics avail;          /**< Availability derived metrics. */
    bool noEphemeris = false;           /**< True when QC ran without ephemeris (elevation outputs omitted). */
};

/** Cycle-slip level: 1 (best) .. 4 (worst). */
enum class SlipLevel   { Level1 = 1, Level2, Level3, Level4 };
/** SNR level: 1 (best) .. 4 (worst). */
enum class SnrLevel    { Level1 = 1, Level2, Level3, Level4 };
/** Multipath level: 1 (best) .. 3 (worst). */
enum class MpLevel     { Level1 = 1, Level2, Level3 };
/** Availability level: 1 (best) .. 4 (worst). */
enum class AvailLevel  { Level1 = 1, Level2, Level3, Level4 };

/**
 * @brief Diagnostic outcome for one file.
 *
 * Levels reflect the main template tier; auxiliary flags drive the
 * optional "satellite X slips frequently" / "system Y is weak" remarks
 * that the text builder appends independently of the level.
 */
struct DiagnosticResult
{
    SlipLevel  slipLevel  = SlipLevel::Level4;
    SnrLevel   snrLevel   = SnrLevel::Level4;
    MpLevel    mpLevel    = MpLevel::Level3;
    AvailLevel availLevel = AvailLevel::Level4;

    bool slipHasSevereSat = false;      /**< A satellite exceeds slip-ratio alert. */
    int  slipSevereSatSys = -1;         /**< System enum of slipHasSevereSat satellite. */
    unsigned int slipSevereSatPrn = 0;  /**< PRN of slipHasSevereSat satellite. */
    QString slipSevereSatTimeRange;     /**< Human-readable window start time. */

    bool slipSysOutlier = false;        /**< A system OSR is far below peers. */
    int  slipSysOutlierIndex = -1;      /**< System index of slipSysOutlier. */

    bool snrAbnormalFreq = false;       /**< A frequency SNR is far below peers. */
    int  snrAbnormalFreqIndex = -1;     /**< Frequency index of snrAbnormalFreq. */

    bool mpSysOutlier = false;          /**< A system MP ratio is far below peers. */
    int  mpSysOutlierIndex = -1;        /**< System index of mpSysOutlier. */

    bool availHasOutlierSys = false;    /**< A system use rate is far below peers. */
    int  availOutlierSysIndex = -1;     /**< System index of availHasOutlierSys. */
    bool availHasBreak = false;         /**< At least one break observed. */
    bool availBreakLong = false;        /**< Long break observed. */
};

/**
 * @brief Map a UI system index (SYS_INDEX_*) to QCSysTypeEnum.
 *
 * Single shared mapping consumed by QCDiagnoseData.cpp and
 * QCDiagnoseTextBuilder.cpp. SBAS (SYS_INDEX_SBAS) falls back to
 * QCSysComplex because QCSysTypeEnum has no dedicated SBAS slot.
 */
inline QCResultManager::QCSysTypeEnum sysIndexToEnum(int sysIndex)
{
    switch (sysIndex)
    {
        case SYS_INDEX_GPS:  return QCResultManager::QCSysGPS;
        case SYS_INDEX_GLO:  return QCResultManager::QCSysGLO;
        case SYS_INDEX_GAL:  return QCResultManager::QCSysGAL;
        case SYS_INDEX_QZSS: return QCResultManager::QCSysQZSS;
        case SYS_INDEX_BDS:  return QCResultManager::QCSysBDS;
        case SYS_INDEX_XW1:  return QCResultManager::QCSysXW1;
        default:             return QCResultManager::QCSysComplex;
    }
}

/**
 * @brief Build DiagnosticInput from raw QC result.
 *
 * Reads QC_RESULT_T via QCResultManager, computes derived metrics, and
 * filters lower-layer -1.0 sentinels. The fileIndex is normally
 * kObsFileIndex (rover only).
 *
 * @param[in] result      Raw QC result struct.
 * @param[in] mgr         Result manager (used for time conversion helpers).
 * @param[in] fileIndex   File slot to read; defaults handled by caller.
 * @param[in] noEphemeris True when QC ran without ephemeris; surfaces the
 *                        no-ephemeris notice in text and HTML banner output.
 * @return Filled DiagnosticInput.
 */
DiagnosticInput buildDiagnosticInput(const QC_RESULT_T &result,
                                     const QCResultManager &mgr,
                                     int fileIndex,
                                     bool noEphemeris = false);

} // namespace QCDiag

#endif // QCDIAGNOSEDATA_H
