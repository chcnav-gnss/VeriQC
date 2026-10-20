/**********************************************************************//**
 * 		VeriQC
 *
 * 		UI Diagnostic Engine Module
 * *-
 * @file QCDiagnoseEngine.cpp
 * @author CHC
 * @date 2026-06-15
 * @brief Implements level-mapping logic for the four diagnostic
 *        dimensions. See header for the overall contract.
 *
 **************************************************************************/

#include "QCDiagnoseEngine.h"
#include "QCDiagnoseConstants.h"
#include "VeriQC.h"
#include <limits>

namespace QCDiag
{

void DiagnoseEngine::diagnose(const DiagnosticInput &input, DiagnosticResult &out)
{
    out = DiagnosticResult();   // reset to defaults
    out.slipLevel  = diagnoseSlip(input.slip, out);
    out.snrLevel   = diagnoseSnr(input.snr, out);
    out.mpLevel    = diagnoseMp(input.mp, out);
    out.availLevel = diagnoseAvailability(input.avail, out);
}

SlipLevel DiagnoseEngine::diagnoseSlip(const SlipMetrics &m, DiagnosticResult &out)
{
    using namespace QCDiagConst;
    for (int lv = 0; lv < 4; ++lv)
    {
        const SlipThreshold &t = kSlipThresholds[lv];
        if (m.slipEpochRatioOverall < t.maxSlipEpochRatio &&
            m.avgHealthyArcRatio    > t.minAvgHealthyArcRatio &&
            m.maxHealthyArcRatio    > t.minMaxHealthyArcRatio &&
            m.avgOSR                > t.minAvgOSR)
        {
            decorateSlipAux(m, out);
            return static_cast<SlipLevel>(lv + 1);
        }
    }
    decorateSlipAux(m, out);
    return SlipLevel::Level4;   // fallback; unreachable due to L4 always-match
}

SnrLevel DiagnoseEngine::diagnoseSnr(const SnrMetrics &m, DiagnosticResult &out)
{
    using namespace QCDiagConst;
    // Average SNR across all valid entries is used for level comparison.
    double sumSnr = 0.0;
    int countSnr = 0;
    for (int sys = 0; sys < QC_MAX_NUM_SYS; ++sys)
    {
        for (int freq = 0; freq < QC_NUM_FREQ; ++freq)
        {
            if (m.avgSNR[sys][freq] > 0.0)
            {
                sumSnr += m.avgSNR[sys][freq];
                ++countSnr;
            }
        }
    }
    const double avgSnr = (countSnr > 0) ? (sumSnr / countSnr) : 0.0;

    for (int lv = 0; lv < 4; ++lv)
    {
        const SnrThreshold &t = kSnrThresholds[lv];
        if (avgSnr > t.minAvgSnr && m.lowCN0SatRatio < t.maxLowCN0SatRatio)
        {
            decorateSnrAux(m, out);
            return static_cast<SnrLevel>(lv + 1);
        }
    }
    decorateSnrAux(m, out);
    return SnrLevel::Level4;
}

MpLevel DiagnoseEngine::diagnoseMp(const MpMetrics &m, DiagnosticResult &out)
{
    using namespace QCDiagConst;
    // Take the worst cross-frequency ratio from MPGroupPercentAve.
    double worst03 = std::numeric_limits<double>::max();
    double worst06 = std::numeric_limits<double>::max();
    double worst09 = std::numeric_limits<double>::max();
    bool hasValid = false;
    for (int freq = 0; freq < QC_NUM_FREQ; ++freq)
    {
        float v03 = m.groupRatioAve[freq][kMpGroupIdx03];
        float v06 = m.groupRatioAve[freq][kMpGroupIdx06];
        float v09 = m.groupRatioAve[freq][kMpGroupIdx09];
        if (v03 < 0.0f || v06 < 0.0f || v09 < 0.0f)
        {
            continue;
        }
        hasValid = true;
        worst03 = std::min(worst03, static_cast<double>(v03));
        worst06 = std::min(worst06, static_cast<double>(v06));
        worst09 = std::min(worst09, static_cast<double>(v09));
    }
    if (!hasValid)
    {
        // No valid MP observation at all; fall back to the worst level.
        decorateMpAux(m, out);
        return MpLevel::Level3;
    }

    for (int lv = 0; lv < 3; ++lv)
    {
        const MpThreshold &t = kMpThresholds[lv];
        if (worst03 > t.minRatioLt03 &&
            worst06 > t.minRatioLt06 &&
            worst09 > t.minRatioLt09)
        {
            decorateMpAux(m, out);
            return static_cast<MpLevel>(lv + 1);
        }
    }
    decorateMpAux(m, out);
    return MpLevel::Level3;
}

AvailLevel DiagnoseEngine::diagnoseAvailability(const AvailabilityMetrics &m, DiagnosticResult &out)
{
    using namespace QCDiagConst;
    decorateAvailabilityAux(m, out);

    const bool hasOutlierSys = (m.outlierSysIndex >= 0);
    const bool hasLongBreak = out.availBreakLong;
    const bool hasFrequentBreaks = (m.obsBreakCount >= kAvailabilityThresholds.breakCountModerate);
    const bool hasFewBreaks = (m.obsBreakCount >= kAvailabilityThresholds.breakCountMinor) &&
                              (m.obsBreakCount < kAvailabilityThresholds.breakCountModerate);

    // Level mapping:
    //   L1: no outlier sys, no breaks.
    //   L2: outlier sys present, but no breaks (or only short infrequent ones).
    //   L3: a few short breaks (1..2), no long break.
    //   L4: long break or frequent breaks (>=3).
    if (hasLongBreak || hasFrequentBreaks)
    {
        return AvailLevel::Level4;
    }
    if (hasFewBreaks)
    {
        return AvailLevel::Level3;
    }
    if (hasOutlierSys)
    {
        return AvailLevel::Level2;
    }
    return AvailLevel::Level1;
}

void DiagnoseEngine::decorateSlipAux(const SlipMetrics &m, DiagnosticResult &out)
{
    using namespace QCDiagConst;
    // Pick the worst window by outLimitEpochRatio (tie-break by
    // outLimitEpochNum). Previously the first hasSevereSat window was taken,
    // which could highlight an early minor window over the truly concentrated
    // one. The burst time is always filled from this worst window so the text
    // builder reports the most severe interval; the per-satellite severe
    // annotation is emitted only when the worst window itself has one, to
    // avoid stitching time and satellite from different windows.
    int worstIdx = -1;
    double worstRatio = -1.0;
    unsigned int worstEpochs = 0;
    for (int i = 0; i < m.outlierWindows.size(); ++i)
    {
        const SlipMetrics::SlipWindowSummary &w = m.outlierWindows.at(i);
        const bool better = (w.outLimitEpochRatio > worstRatio) ||
            (w.outLimitEpochRatio == worstRatio && w.outLimitEpochNum > worstEpochs);
        if (better)
        {
            worstRatio = w.outLimitEpochRatio;
            worstEpochs = w.outLimitEpochNum;
            worstIdx = i;
        }
    }
    if (worstIdx >= 0)
    {
        const SlipMetrics::SlipWindowSummary &w = m.outlierWindows.at(worstIdx);
        out.slipSevereSatTimeRange = w.startTime.toString("yyyy/MM/dd HH:mm");
        if (w.hasSevereSat && w.worstSatIndex >= 0)
        {
            int32_t prn = 0;
            int32_t sysFlag = GetQCSatSys(static_cast<int32_t>(w.worstSatIndex + 1), &prn);
            out.slipHasSevereSat = true;
            out.slipSevereSatPrn = static_cast<unsigned int>(prn);
            // Map QC_SYS_* bitmask to system index (0..6).
            switch (sysFlag)
            {
                case QC_SYS_GPS: out.slipSevereSatSys = SYS_INDEX_GPS; break;
                case QC_SYS_SBS: out.slipSevereSatSys = SYS_INDEX_SBAS; break;
                case QC_SYS_GLO: out.slipSevereSatSys = SYS_INDEX_GLO; break;
                case QC_SYS_GAL: out.slipSevereSatSys = SYS_INDEX_GAL; break;
                case QC_SYS_QZS: out.slipSevereSatSys = SYS_INDEX_QZSS; break;
                case QC_SYS_CMP:
                case QC_SYS_BD3: out.slipSevereSatSys = SYS_INDEX_BDS; break;
                case QC_SYS_XW1: out.slipSevereSatSys = SYS_INDEX_XW1; break;
                default: out.slipSevereSatSys = -1; break;
            }
        }
    }

    // System outlier: a system whose OSR is far below the peer average.
    int validCount = 0;
    double sumValid = 0.0;
    for (int sys = 0; sys < QC_MAX_NUM_SYS; ++sys)
    {
        if (m.sysOSR[sys] < QCDiagConst::kOSRInvalid)
        {
            sumValid += m.sysOSR[sys];
            ++validCount;
        }
    }
    if (validCount >= 2)
    {
        const double peerAvg = sumValid / validCount;
        for (int sys = 0; sys < QC_MAX_NUM_SYS; ++sys)
        {
            if (m.sysOSR[sys] < QCDiagConst::kOSRInvalid &&
                m.sysOSR[sys] < peerAvg * kSlipSysOSRDiffRatio)
            {
                out.slipSysOutlier = true;
                out.slipSysOutlierIndex = sys;
                break;
            }
        }
    }
}

void DiagnoseEngine::decorateSnrAux(const SnrMetrics &m, DiagnosticResult &out)
{
    if (m.worstFreqIndex >= 0)
    {
        out.snrAbnormalFreq = true;
        out.snrAbnormalFreqIndex = m.worstFreqIndex;
    }
}

void DiagnoseEngine::decorateMpAux(const MpMetrics &m, DiagnosticResult &out)
{
    if (m.worstSysIndex >= 0)
    {
        out.mpSysOutlier = true;
        out.mpSysOutlierIndex = m.worstSysIndex;
    }
}

void DiagnoseEngine::decorateAvailabilityAux(const AvailabilityMetrics &m, DiagnosticResult &out)
{
    if (m.outlierSysIndex >= 0)
    {
        out.availHasOutlierSys = true;
        out.availOutlierSysIndex = m.outlierSysIndex;
    }
    if (m.obsBreakCount > 0 && !m.topBreaks.isEmpty())
    {
        out.availHasBreak = true;
        const AvailabilityMetrics::BreakSummary &longest = m.topBreaks.first();
        out.availBreakLong = (longest.durationSec >
                              static_cast<float>(QCDiagConst::kAvailabilityThresholds.breakDurationLongSec));
    }
}

} // namespace QCDiag
