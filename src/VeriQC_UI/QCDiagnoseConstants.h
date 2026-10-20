/**********************************************************************//**
 * 		VeriQC
 *
 * 		UI Diagnostic Constants Module
 * *-
 * @file QCDiagnoseConstants.h
 * @author CHC
 * @date 2026-06-15
 * @brief Threshold constants for data anomaly diagnostic summary.
 *
 * Centralizes every tunable threshold used by the diagnostic engine so
 * that level-mapping rules can be tuned in one place. All values are
 * compile-time constants to enable optimizer folding and to avoid static
 * initialization order issues.
 *
 **************************************************************************/
#ifndef QCDIAGNOSECONSTANTS_H
#define QCDIAGNOSECONSTANTS_H


/** @brief Index type used for MP group enumeration. */
typedef unsigned int MpGroupIndex;

/**
 * @namespace QCDiagConst
 * @brief Aggregates all threshold constants for the four diagnostic
 *        dimensions (cycle slip, SNR, multipath, availability).
 *
 * Value ranges:
 * - Ratios (slipEpochRatio, healthyArcRatio, lowCN0SatRatio, mpGroup*)
 *   are stored as fractions in [0.0, 1.0]. The lower-layer writes -1.0
 *   when the denominator is zero; callers must filter negative values
 *   before comparing with these thresholds.
 * - Durations are expressed in seconds.
 * - SNR/CN0 values are expressed in dB-Hz.
 * - OSR (Overall Slip Ratio = detect/slip) is a positive scalar; the
 *   sentinel 99999.0 indicates "no slip detected" (mirrors
 *   QCResultManager::calculateSysCycleJumpRatio).
 */
namespace QCDiagConst
{
/** Observatory file index used by UI; the rover is always slot 0. */
constexpr int kObsFileIndex = 0;

/** Sentinel value indicating "no cycle slip observed". */
constexpr double kOSRInvalid = 99999.0;

/**
 * @struct SlipThreshold
 * @brief Level-mapping thresholds for the cycle-slip dimension.
 *
 * A file is classified as Level (N+1) when every condition holds:
 *   slipEpochRatioOverall  < maxSlipEpochRatio     AND
 *   avgHealthyArcRatio     > minAvgHealthyArcRatio AND
 *   maxHealthyArcRatio     > minMaxHealthyArcRatio AND
 *   avgOSR                 > minAvgOSR.
 *
 * Levels are evaluated from best (Level 1) to worst (Level 4); the
 * first fully-matching level wins. The Level 4 entry is a fallback
 * that always matches.
 */
struct SlipThreshold
{
    double maxSlipEpochRatio;     /**< Upper bound of outlier-epoch ratio. */
    double minAvgHealthyArcRatio; /**< Lower bound of avg healthy arc ratio. */
    double minMaxHealthyArcRatio; /**< Lower bound of max healthy arc ratio. */
    double minAvgOSR;             /**< Lower bound of overall slip ratio. */
};

/** Cycle-slip threshold table; index = level - 1. */
constexpr SlipThreshold kSlipThresholds[4] = {
    { 0.05, 0.90, 0.95, 3000.0 },   /**< Level 1: generally good. */
    { 0.20, 0.80, 0.80, 1000.0 },   /**< Level 2: minor issues on a few sats. */
    { 0.50, 0.50, 0.50,  600.0 },   /**< Level 3: frequent slips, fair stability. */
    { 1.01, 0.00, 0.00,    0.0 }    /**< Level 4: fallback, severe slips. */
};

/** Satellite slip-ratio alert inside an outlier window (60% in spec). */
constexpr double kSlipSatRatioAlert        = 0.60;

/** A system is flagged as outlier when its OSR is below this fraction of the peer average. */
constexpr double kSlipSysOSRDiffRatio      = 0.50;

/** Average healthy-arc duration threshold for the "less than 10 minutes" remark. */
constexpr double kSlipAvgHealthyArcMinSec  = 600.0;

/**
 * @struct SnrThreshold
 * @brief Level-mapping thresholds for the SNR dimension.
 *
 * Level (N+1) is selected when:
 *   avgSNR           > minAvgSnr          AND
 *   lowCN0SatRatio   < maxLowCN0SatRatio.
 */
struct SnrThreshold
{
    double minAvgSnr;            /**< Lower bound of average SNR (dB-Hz). */
    double maxLowCN0SatRatio;    /**< Upper bound of low-CN0 satellite ratio. */
};

/** SNR threshold table; index = level - 1. */
constexpr SnrThreshold kSnrThresholds[4] = {
    { 40.0, 0.20 },              /**< Level 1: overall good. */
    { 35.0, 1.01 },              /**< Level 2: weak signal locally (no ratio cap). */
    {  0.0, 1.01 },              /**< Level 3: weak signal overall. */
    { -1.0, 1.01 }               /**< Level 4: fallback, poor quality. */
};

/** Low-CN0 satellite-ratio threshold for the "severe" branch (40% in spec). */
constexpr double kLowCN0SatRatioSevere = 0.40;

/** Frequency-anomaly drop in dB (a frequency is flagged when > 10 dB below peers). */
constexpr double kSnrFreqDropDb        = 10.0;

/** CN0 drop-window alert in dB (a window is flagged when drop > 5 dB). */
constexpr double kSnrWindowDropDb      = 5.0;

/**
 * @struct MpThreshold
 * @brief Level-mapping thresholds for the multipath dimension.
 *
 * Level (N+1) is selected when the worst cross-frequency ratio (taken
 * from MPGroupPercentAve) simultaneously satisfies all three bounds:
 *   ratioLt03 > minRatioLt03 AND
 *   ratioLt06 > minRatioLt06 AND
 *   ratioLt09 > minRatioLt09.
 */
struct MpThreshold
{
    double minRatioLt03;   /**< Lower bound of ratio with MP < 0.3 m. */
    double minRatioLt06;   /**< Lower bound of ratio with MP < 0.6 m. */
    double minRatioLt09;   /**< Lower bound of ratio with MP < 0.9 m. */
};

/** Multipath threshold table; index = level - 1. */
constexpr MpThreshold kMpThresholds[3] = {
    { 0.60, 0.90, 0.97 },        /**< Level 1: stable multipath. */
    { 0.40, 0.80, 0.90 },        /**< Level 2: noticeable multipath. */
    { 0.00, 0.00, 0.00 }         /**< Level 3: fallback, severe multipath. */
};

/** MP group indices into the third dimension of MPGroupPercent[Ave]. */
constexpr MpGroupIndex kMpGroupIdx03 = 0;   /**< < 0.3 m. */
constexpr MpGroupIndex kMpGroupIdx06 = 1;   /**< < 0.6 m. */
constexpr MpGroupIndex kMpGroupIdx09 = 2;   /**< < 0.9 m. */

/** A system is flagged as the worst MP system when its <0.3m ratio is below 40%. */
constexpr double kMpWorstSysRatioLt03Threshold = 0.40;

/**
 * @struct AvailabilityThreshold
 * @brief Level-mapping thresholds for the availability dimension.
 *
 * The availability level is derived from a combination of system-level
 * use-rate divergence and observation-break statistics rather than a
 * simple monotonic table; see QCDiagnoseEngine for the case analysis.
 */
struct AvailabilityThreshold
{
    double useRateSysDiffAlert;   /**< System-use-rate divergence threshold. */
    int    breakCountMinor;       /**< Lower bound of "few short breaks". */
    int    breakCountModerate;    /**< Lower bound of "frequent breaks". */
    double breakDurationLongSec;  /**< Break-duration threshold for "long break". */
};

constexpr AvailabilityThreshold kAvailabilityThresholds = {
    0.20,   /**< A system is flagged when its use rate is 20%+ below peers. */
    1,      /**< Break count >= 1 (combined with < 3) triggers "few breaks". */
    3,      /**< Break count >= 3 triggers "frequent breaks". */
    300.0   /**< Break duration > 5 minutes triggers "long break". */
};

} // namespace QCDiagConst

#endif // QCDIAGNOSECONSTANTS_H
