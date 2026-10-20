/**********************************************************************//**
 * 		VeriQC
 *
 * 		UI Diagnostic Data Module
 * *-
 * @file QCDiagnoseData.cpp
 * @author CHC
 * @date 2026-06-15
 * @brief Implements buildDiagnosticInput that derives normalized metrics
 *        from the raw QC_RESULT_T.
 *
 **************************************************************************/

#include "QCDiagnoseData.h"
#include "QCDiagnoseConstants.h"
#include <algorithm>
#include <limits>

namespace QCDiag
{

/**
 * @brief Convert a lower-layer ratio into a safe value.
 *
 * The lower layer writes -1.0 when the denominator is zero; this helper
 * folds such sentinels to 0.0 so the engine can compare without extra
 * branching.
 */
static inline double sanitizeRatio(float v)
{
    return (v < 0.0f) ? 0.0 : static_cast<double>(v);
}

/**
 * @brief Compute the worst satellite inside a slip-outlier window.
 *
 * @param[in]  window    Raw window info from QC_RESULT_T.
 * @param[out] summary   Filled summary; worstSatIndex remains -1 when no
 *                       satellite exceeds kSlipSatRatioAlert.
 */
static void fillSlipWindowSummary(const QC_SLIP_TIME_WIN_INFO_T &window,
                                  SlipMetrics::SlipWindowSummary &summary)
{
    summary.startTime = QCResultManager::qcTime2DateTime(window.WindowStartTime);
    summary.epochNum = window.WindowEpochNum;
    summary.outLimitEpochNum = window.OutLimitEpochNum;
    summary.outLimitEpochRatio = window.OutLimitEpochRatio;

    int worstIdx = -1;
    double worstRatio = 0.0;
    for (int sat = 0; sat < QC_MAX_NUM_SAT; ++sat)
    {
        if (window.SatNum[sat] == 0)
        {
            continue;
        }
        double ratio = static_cast<double>(window.SatSlipNum[sat]) /
                       static_cast<double>(window.SatNum[sat]);
        if (ratio > worstRatio)
        {
            worstRatio = ratio;
            worstIdx = sat;
        }
    }
    summary.worstSatIndex = worstIdx;
    summary.worstSatSlipRatio = worstRatio;
    summary.hasSevereSat = (worstIdx >= 0) &&
                           (worstRatio >= QCDiagConst::kSlipSatRatioAlert);
}

/**
 * @brief Derive SlipMetrics from QC_RESULT_T.
 */
static void buildSlipMetrics(const QC_RESULT_T &result,
                             const QCResultManager &mgr,
                             int fileIndex,
                             SlipMetrics &out)
{
    const QDateTime startDt = mgr.getStartTime();
    const QDateTime endDt = mgr.getEndTime();
    if (startDt.isValid() && endDt.isValid())
    {
        qint64 secs = startDt.secsTo(endDt);
        out.totalDurationSec = (secs > 0) ? static_cast<double>(secs) : 0.0;
    }

    out.avgHealthyArcDurationSec = result.AvgHealthyArcDurationSec[fileIndex];
    out.maxHealthyArcDurationSec = result.MaxHealthyArcDurationSec[fileIndex];
    if (out.totalDurationSec > 0.0)
    {
        out.avgHealthyArcRatio = out.avgHealthyArcDurationSec / out.totalDurationSec;
        out.maxHealthyArcRatio = out.maxHealthyArcDurationSec / out.totalDurationSec;
    }

    // Aggregate slip-epoch ratio across all valid outlier windows.
    unsigned long long totalOut = 0;
    unsigned long long totalEpoch = 0;
    int winCount = result.SlipOutlierWindowCount[fileIndex];
    if (winCount < 0)
    {
        winCount = 0;
    }
    if (winCount > QC_DIAG_TOP_WIN_NUM)
    {
        winCount = QC_DIAG_TOP_WIN_NUM;
    }
    out.outlierWindows.clear();
    for (int i = 0; i < winCount; ++i)
    {
        const QC_SLIP_TIME_WIN_INFO_T &w = result.SlipOutlierWindow[fileIndex][i];
        SlipMetrics::SlipWindowSummary summary;
        fillSlipWindowSummary(w, summary);
        out.outlierWindows.append(summary);
        totalOut += w.OutLimitEpochNum;
        totalEpoch += w.WindowEpochNum;
    }
    out.slipEpochRatioOverall = (totalEpoch > 0)
        ? static_cast<double>(totalOut) / static_cast<double>(totalEpoch)
        : 0.0;

    /** Compute avgOSR as mean of per-system OSR values.
     *  Systems without slips (sentinel kOSRInvalid) are excluded
     *  from the average. Falls back to kOSRInvalid when no system has slips. */
    out.avgOSR = QCDiagConst::kOSRInvalid;
    int validSysCount = 0;
    double sumOSR = 0.0;
    for (int sys = 0; sys < QC_MAX_NUM_SYS; ++sys)
    {
        int slipNum = result.SysSlipNum[fileIndex][sys];
        int detectNum = result.SysDetectNum[fileIndex][sys];
        if (slipNum > 0 && detectNum > 0)
        {
            double sysOSR = static_cast<double>(detectNum) /
                           static_cast<double>(slipNum);
            out.sysOSR[sys] = sysOSR;
            sumOSR += sysOSR;
            ++validSysCount;
        }
        else
        {
            out.sysOSR[sys] = QCDiagConst::kOSRInvalid;
        }
    }
    if (validSysCount > 0)
    {
        out.avgOSR = sumOSR / validSysCount;
    }
}

/**
 * @brief Derive SnrMetrics from QC_RESULT_T.
 */
static void buildSnrMetrics(const QC_RESULT_T &result,
                            int fileIndex,
                            SnrMetrics &out)
{
    out.lowCN0SatRatio = sanitizeRatio(result.LowCN0SatRatio[fileIndex]);
    out.lowCN0EpochRatio = sanitizeRatio(result.LowCN0EpochRatio[fileIndex]);

    for (int sys = 0; sys < QC_MAX_NUM_SYS; ++sys)
    {
        for (int freq = 0; freq < QC_NUM_FREQ; ++freq)
        {
            out.avgSNR[sys][freq] = static_cast<double>(result.SNR[fileIndex][sys][freq]);
        }
    }

    // Identify the worst frequency, averaged across systems, when it is
    // more than kSnrFreqDropDb below the peer mean.
    double freqMean[QC_NUM_FREQ] = {0};
    int freqValidCount[QC_NUM_FREQ] = {0};
    for (int freq = 0; freq < QC_NUM_FREQ; ++freq)
    {
        double sum = 0.0;
        int count = 0;
        for (int sys = 0; sys < QC_MAX_NUM_SYS; ++sys)
        {
            if (out.avgSNR[sys][freq] > 0.0)
            {
                sum += out.avgSNR[sys][freq];
                ++count;
            }
        }
        if (count > 0)
        {
            freqMean[freq] = sum / count;
            freqValidCount[freq] = count;
        }
    }

    int validFreqTotal = 0;
    double grandSum = 0.0;
    for (int freq = 0; freq < QC_NUM_FREQ; ++freq)
    {
        if (freqValidCount[freq] > 0)
        {
            grandSum += freqMean[freq];
            ++validFreqTotal;
        }
    }
    if (validFreqTotal >= 2)
    {
        const double peerMean = grandSum / validFreqTotal;
        int worstFreq = -1;
        double worstDrop = 0.0;
        for (int freq = 0; freq < QC_NUM_FREQ; ++freq)
        {
            if (freqValidCount[freq] == 0)
            {
                continue;
            }
            double drop = peerMean - freqMean[freq];
            if (drop > QCDiagConst::kSnrFreqDropDb && drop > worstDrop)
            {
                worstDrop = drop;
                worstFreq = freq;
            }
        }
        out.worstFreqIndex = worstFreq;
        out.worstFreqAvgDropDb = worstDrop;
    }

    // CN0 drop windows.
    int dropWinCount = result.CN0DropWindowCount[fileIndex];
    if (dropWinCount < 0)
    {
        dropWinCount = 0;
    }
    if (dropWinCount > QC_DIAG_TOP_WIN_NUM)
    {
        dropWinCount = QC_DIAG_TOP_WIN_NUM;
    }
    out.dropWindows.clear();
    out.alertDropWindowCount = 0;
    for (int i = 0; i < dropWinCount; ++i)
    {
        const QC_CN0_TIME_WIN_INFO_T &w = result.CN0DropWindow[fileIndex][i];
        SnrMetrics::CN0DropWindowSummary summary;
        summary.startTime = QCResultManager::qcTime2DateTime(w.WindowStartTime);
        summary.startAvgCN0 = w.WindowStartAvgCN0;
        summary.avgCN0Drop = w.AvgCN0Drop;
        summary.isAlert = (w.AvgCN0Drop > static_cast<float>(QCDiagConst::kSnrWindowDropDb));
        if (summary.isAlert)
        {
            ++out.alertDropWindowCount;
        }
        out.dropWindows.append(summary);
    }
}

/**
 * @brief Derive MpMetrics from QC_RESULT_T.
 */
static void buildMpMetrics(const QC_RESULT_T &result,
                           int fileIndex,
                           MpMetrics &out)
{
    for (int sys = 0; sys < QC_MAX_NUM_SYS; ++sys)
    {
        for (int freq = 0; freq < QC_NUM_FREQ; ++freq)
        {
            for (int g = 0; g < MP_GROUP_PERCENT_SIZE; ++g)
            {
                out.groupRatio[sys][freq][g] = result.MPGroupPercent[fileIndex][sys][freq][g];
            }
        }
    }
    for (int freq = 0; freq < QC_NUM_FREQ; ++freq)
    {
        for (int g = 0; g < MP_GROUP_PERCENT_SIZE; ++g)
        {
            out.groupRatioAve[freq][g] = result.MPGroupPercentAve[fileIndex][freq][g];
        }
    }

    // Worst frequency = lowest valid <0.3m ratio across MPGroupPercentAve.
    int worstFreq = -1;
    float worstFreqRatio = std::numeric_limits<float>::max();
    for (int freq = 0; freq < QC_NUM_FREQ; ++freq)
    {
        float v = out.groupRatioAve[freq][QCDiagConst::kMpGroupIdx03];
        if (v >= 0.0f && v < worstFreqRatio)
        {
            worstFreqRatio = v;
            worstFreq = freq;
        }
    }
    out.worstFreqIndex = worstFreq;

    // Worst system = lowest mean <0.3m ratio across frequencies, when below 40%.
    int worstSys = -1;
    float worstSysRatio = std::numeric_limits<float>::max();
    for (int sys = 0; sys < QC_MAX_NUM_SYS; ++sys)
    {
        double sum = 0.0;
        int count = 0;
        for (int freq = 0; freq < QC_NUM_FREQ; ++freq)
        {
            float v = out.groupRatio[sys][freq][QCDiagConst::kMpGroupIdx03];
            if (v >= 0.0f)
            {
                sum += v;
                ++count;
            }
        }
        if (count > 0)
        {
            double mean = sum / count;
            if (mean < worstSysRatio)
            {
                worstSysRatio = static_cast<float>(mean);
                worstSys = sys;
            }
        }
    }
    if (worstSys >= 0 &&
        worstSysRatio < static_cast<float>(QCDiagConst::kMpWorstSysRatioLt03Threshold))
    {
        out.worstSysIndex = worstSys;
    }
    else
    {
        out.worstSysIndex = -1;
    }
}

/**
 * @brief Derive AvailabilityMetrics from QC_RESULT_T.
 */
static void buildAvailabilityMetrics(const QC_RESULT_T &result,
                                     const QCResultManager &mgr,
                                     int fileIndex,
                                     AvailabilityMetrics &out)
{
    out.useRateTotal = result.UseRateTotal[fileIndex];

    int validSys[QC_MAX_NUM_SYS] = {0};
    int validCount = 0;
    double sumValid = 0.0;
    int minSys = -1;
    double minValue = std::numeric_limits<double>::max();
    for (int sys = 0; sys < QC_MAX_NUM_SYS; ++sys)
    {
        double rate = result.UseRateSys[fileIndex][sys];
        out.useRateSys[sys] = rate;
        if (rate > 0.0)
        {
            validSys[sys] = 1;
            ++validCount;
            sumValid += rate;
            if (rate < minValue)
            {
                minValue = rate;
                minSys = sys;
            }
        }
    }
    out.sysCount = validCount;

    if (validCount >= 2 && minSys >= 0)
    {
        double peerSum = sumValid - minValue;
        double peerAvg = peerSum / (validCount - 1);
        if ((peerAvg - minValue) > QCDiagConst::kAvailabilityThresholds.useRateSysDiffAlert)
        {
            out.outlierSysIndex = minSys;
            out.outlierSysUseRate = minValue;
            out.peerAvgUseRate = peerAvg;
        }
    }

    out.obsBreakCount = result.ObsBreakCount[fileIndex];
    if (out.obsBreakCount < 0)
    {
        out.obsBreakCount = 0;
    }

    int breakTopCount = out.obsBreakCount;
    if (breakTopCount > QC_DIAG_TOP_BREAK_NUM)
    {
        breakTopCount = QC_DIAG_TOP_BREAK_NUM;
    }
    out.topBreaks.clear();
    for (int i = 0; i < breakTopCount; ++i)
    {
        const QC_OBS_BREAK_INFO_T &b = result.ObsBreakTop[fileIndex][i];
        AvailabilityMetrics::BreakSummary bs;
        bs.startTime = QCResultManager::qcTime2DateTime(b.BreakStartTime);
        bs.durationSec = b.BreakDurationSec;
        out.topBreaks.append(bs);
    }

    // FR-09: flag systems that were observed (at least one valid PRN) but
    // report zero use rate. getValidSysSatPrns is the most reliable
    // "system was observed" signal available without lower-layer changes;
    // SBAS maps to QCSysComplex and yields an empty list, so it stays false.
    for (int sys = 0; sys < QC_MAX_NUM_SYS; ++sys)
    {
        const QCResultManager::QCSysTypeEnum sysEnum = sysIndexToEnum(sys);
        if (sysEnum == QCResultManager::QCSysComplex)
        {
            out.sysObserved[sys] = false;
            continue;
        }
        out.sysObserved[sys] = mgr.hasValidSysSatPrns(sysEnum);
    }
}

DiagnosticInput buildDiagnosticInput(const QC_RESULT_T &result,
                                     const QCResultManager &mgr,
                                     int fileIndex,
                                     bool noEphemeris)
{
    DiagnosticInput input;
    input.noEphemeris = noEphemeris;
    input.startTime = mgr.getStartTime();
    input.endTime = mgr.getEndTime();

    buildSlipMetrics(result, mgr, fileIndex, input.slip);
    buildSnrMetrics(result, fileIndex, input.snr);
    buildMpMetrics(result, fileIndex, input.mp);
    buildAvailabilityMetrics(result, mgr, fileIndex, input.avail);

    return input;
}

} // namespace QCDiag
