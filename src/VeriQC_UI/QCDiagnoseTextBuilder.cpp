/**********************************************************************//**
 * 		VeriQC
 *
 * 		UI Diagnostic Text Builder Module
 * *-
 * @file QCDiagnoseTextBuilder.cpp
 * @author CHC
 * @date 2026-06-15
 * @brief Localized template instantiation for the diagnostic summary.
 *
 * The Chinese translations live in VeriQC_UI_zh_CN.ts under the
 * "QCDiagnoseTextBuilder" context.
 *
 **************************************************************************/

#include "QCDiagnoseTextBuilder.h"
#include "QCResultManager.h"
#include <QCoreApplication>
#include <QSet>

namespace QCDiag
{

QString QCDiagnoseTextBuilder::buildFullText(const DiagnosticInput &input,
                                             const DiagnosticResult &result,
                                             const QCResultManager &mgr)
{
    QStringList lines;
    if (input.noEphemeris)
    {
        lines << QCoreApplication::translate("QCDiagnoseTextBuilder",
                    "Ephemeris unavailable - elevation-angle data is unavailable; remarks referencing elevation angles are omitted. Slip, SNR, multipath and availability conclusions remain based on observations.");
        lines << QString();
    }
    lines << QCoreApplication::translate("QCDiagnoseTextBuilder",
                "Cycle Slip Diagnosis");
    lines.append(buildSlipSection(input, result, mgr));
    lines << QString();
    lines << QCoreApplication::translate("QCDiagnoseTextBuilder",
                "SNR Diagnosis");
    lines.append(buildSnrSection(input, result));
    lines << QString();
    lines << QCoreApplication::translate("QCDiagnoseTextBuilder",
                "Multipath Diagnosis");
    lines.append(buildMpSection(input, result));
    lines << QString();
    lines << QCoreApplication::translate("QCDiagnoseTextBuilder",
                "Availability Diagnosis");
    lines.append(buildAvailabilitySection(input, result, mgr));
    return lines.join("\n");
}

/**
 * @brief Resolve the best-available burst time string for slip-diagnosis text.
 *
 * Prefers the engine-computed slipSevereSatTimeRange; falls back to the
 * start time of the first outlier window when the engine result is empty.
 */
static QString resolveBurstTime(const DiagnosticResult &result, const SlipMetrics &m)
{
    Q_UNUSED(m);
    // decorateSlipAux always fills slipSevereSatTimeRange when outlierWindows
    // is non-empty (worstIdx >= 0), so a fallback rescan here would be
    // unreachable. When outlierWindows is empty, slipSevereSatTimeRange is
    // empty too and callers render the localized "affected intervals" text.
    // Returning the engine-computed value keeps a single source of truth for
    // the burst time and avoids a duplicated selection loop.
    return result.slipSevereSatTimeRange;
}

QStringList QCDiagnoseTextBuilder::buildSlipSection(const DiagnosticInput &input,
                                                    const DiagnosticResult &result,
                                                    const QCResultManager &mgr)
{
    Q_UNUSED(mgr);
    QStringList paragraphs;
    const SlipMetrics &m = input.slip;
    const double avgArcSeconds = m.avgHealthyArcDurationSec;
    const double avgArcPct = m.avgHealthyArcRatio * 100.0;

    switch (result.slipLevel)
    {
        case SlipLevel::Level1:
        {
            paragraphs << QCoreApplication::translate("QCDiagnoseTextBuilder",
                "Cycle slip condition is generally good. During the observation period "
                "the cycle-slip rate is low: slip-epoch ratio is below 5%, and the "
                "average healthy arc covers %1 of the total duration "
                "(%2 seconds on average).")
                .arg(QString::number(avgArcPct, 'f', 1) + '%')
                .arg(avgArcSeconds, 0, 'f', 0);
            break;
        }
        case SlipLevel::Level2:
        {
            paragraphs << QCoreApplication::translate("QCDiagnoseTextBuilder",
                "Most observation arcs remain continuous; a few satellites may be "
                "affected by low-elevation masking or marginal signal quality.");
            if (result.slipHasSevereSat && result.slipSevereSatSys >= 0)
            {
                const QCResultManager::QCSysTypeEnum sysEnum = sysIndexToEnum(result.slipSevereSatSys);
                QString satName;
                if (sysEnum != QCResultManager::QCSysComplex)
                {
                    satName = QCResultManager::getSatName(sysEnum, result.slipSevereSatPrn);
                }
                else
                {
                    satName = QCoreApplication::translate("QCDiagnoseTextBuilder", "N/A");
                }
                paragraphs << QCoreApplication::translate("QCDiagnoseTextBuilder",
                    "Satellite %1 slips frequently; during the %2 window its slip ratio "
                    "exceeds 60%.")
                    .arg(satName)
                    .arg(result.slipSevereSatTimeRange);
            }
            break;
        }
        case SlipLevel::Level3:
        {
            const QString burstTime = resolveBurstTime(result, m);
            paragraphs << QCoreApplication::translate("QCDiagnoseTextBuilder",
                "Cycle slips are relatively frequent and continuous-observation "
                "stability is fair. Multiple systems are affected; slips concentrate "
                "around %1. The average continuous healthy arc duration is about "
                "%2 seconds. Possible causes: localized environment changes, temporary "
                "masking, equipment glitches, or enhanced ionospheric disturbances.")
                .arg(burstTime.isEmpty()
                     ? QCoreApplication::translate("QCDiagnoseTextBuilder", "the affected intervals")
                     : burstTime)
                .arg(avgArcSeconds, 0, 'f', 0);
            break;
        }
        case SlipLevel::Level4:
        {
            const QString burstTime = resolveBurstTime(result, m);
            paragraphs << QCoreApplication::translate("QCDiagnoseTextBuilder",
                "Cycle-slip severity is high and observation continuity is insufficient. "
                "Many satellites and a large fraction of epochs are affected, with "
                "concentrated bursts over short intervals. Slips are most frequent "
                "around %1. The average continuous healthy arc duration is about "
                "%2 seconds. Possible causes: receiver-tracking instability, antenna "
                "anomalies, power/cabling issues, or external electromagnetic "
                "interference.")
                .arg(burstTime.isEmpty()
                     ? QCoreApplication::translate("QCDiagnoseTextBuilder", "the affected intervals")
                     : burstTime)
                .arg(avgArcSeconds, 0, 'f', 0);
            break;
        }
    }

    if ((result.slipLevel == SlipLevel::Level3 ||
         result.slipLevel == SlipLevel::Level4) &&
        m.outlierWindows.isEmpty() && m.totalDurationSec > 300.0)
    {
        paragraphs << QCoreApplication::translate("QCDiagnoseTextBuilder",
            "Frequent data packet loss detected; unable to accurately "
            "determine the frequent cycle-slip time intervals.");
    }

    if (result.slipSysOutlier && result.slipSysOutlierIndex >= 0)
    {
        paragraphs << QCoreApplication::translate("QCDiagnoseTextBuilder",
            "System %1 has a notably lower slip ratio compared to peer systems.")
            .arg(systemShortName(result.slipSysOutlierIndex));
    }

    return paragraphs;
}

/**
 * @brief Compute grand-mean SNR across all valid system×frequency cells.
 *
 * Used by both SnrLevel::Level3 and SnrLevel::Level4 diagnostic text;
 * extracted to avoid duplication.
 */
static double computeGrandMeanSNR(const SnrMetrics &m)
{
    double sum = 0.0;
    int count = 0;
    for (int sys = 0; sys < QC_MAX_NUM_SYS; ++sys)
    {
        for (int freq = 0; freq < QC_NUM_FREQ; ++freq)
        {
            if (m.avgSNR[sys][freq] > 0.0)
            {
                sum += m.avgSNR[sys][freq];
                ++count;
            }
        }
    }
    return (count > 0) ? (sum / count) : 0.0;
}

QStringList QCDiagnoseTextBuilder::buildSnrSection(const DiagnosticInput &input,
                                                   const DiagnosticResult &result)
{
    QStringList paragraphs;
    const SnrMetrics &m = input.snr;
    const double lowSatPct = m.lowCN0SatRatio * 100.0;
    const double lowEpochPct = m.lowCN0EpochRatio * 100.0;
    bool showedSharpDrop = false;

    switch (result.snrLevel)
    {
        case SnrLevel::Level1:
        {
            paragraphs << QCoreApplication::translate("QCDiagnoseTextBuilder",
                "Signal quality is overall good. The average SNR during the observation "
                "period exceeds 40 dB-Hz; the low-SNR (<35 dB-Hz) satellite ratio is "
                "%1 and the low-SNR epoch ratio is %2. High-elevation satellites "
                "remain stable.")
                .arg(QString::number(lowSatPct, 'f', 1) + '%')
                .arg(QString::number(lowEpochPct, 'f', 1) + '%');
            break;
        }
        case SnrLevel::Level2:
        {
            paragraphs << QCoreApplication::translate("QCDiagnoseTextBuilder",
                "Signal quality is essentially normal, locally weak. The low-SNR "
                "(<35 dB-Hz) satellite ratio is %1 and the low-SNR epoch ratio is "
                "%2. Weak signals are concentrated at low elevation angles and may "
                "relate to masking edges or multipath noise.")
                .arg(QString::number(lowSatPct, 'f', 1) + '%')
                .arg(QString::number(lowEpochPct, 'f', 1) + '%');
            break;
        }
        case SnrLevel::Level3:
        {
            const double avgSnr = computeGrandMeanSNR(m);
            paragraphs << QCoreApplication::translate("QCDiagnoseTextBuilder",
                "Signal quality is weak and observation stability is fair. The "
                "low-SNR satellite ratio is %1 and the low-SNR epoch ratio is "
                "%2. The average SNR has also declined to %3 dB-Hz, which may "
                "affect carrier tracking and high-precision positioning.")
                .arg(QString::number(lowSatPct, 'f', 1) + '%')
                .arg(QString::number(lowEpochPct, 'f', 1) + '%')
                .arg(avgSnr, 0, 'f', 1);

            if (m.alertDropWindowCount > 0 && !m.dropWindows.isEmpty())
            {
                const SnrMetrics::CN0DropWindowSummary &w = m.dropWindows.first();
                paragraphs << QCoreApplication::translate("QCDiagnoseTextBuilder",
                    "SNR dropped sharply around %1 by %2 dB-Hz, possibly due to "
                    "temporary masking, environmental changes, or instantaneous "
                    "interference.")
                    .arg(w.startTime.toString("yyyy/MM/dd HH:mm"))
                    .arg(static_cast<double>(w.avgCN0Drop), 0, 'f', 1);
                showedSharpDrop = true;
            }
            break;
        }
        case SnrLevel::Level4:
        {
            const double avgSnr = computeGrandMeanSNR(m);
            paragraphs << QCoreApplication::translate("QCDiagnoseTextBuilder",
                "Signal quality is poor; please inspect equipment and environment. "
                "The average SNR is below %1 dB-Hz. The low-SNR observation ratio "
                "exceeds 40%. Multiple systems remain weak for prolonged periods, "
                "suggesting antenna masking, feed-line anomalies, receiver "
                "misconfiguration, or external interference.")
                .arg(avgSnr, 0, 'f', 1);

            if (result.snrAbnormalFreq && result.snrAbnormalFreqIndex >= 0)
            {
                paragraphs << QCoreApplication::translate("QCDiagnoseTextBuilder",
                    "Frequency %1 is notably weaker, with average SNR more than "
                    "10 dB-Hz below other frequencies.")
                    .arg(frequencyLabel(result.snrAbnormalFreqIndex));
            }
            break;
        }
    }

    if (!m.dropWindows.isEmpty() && !showedSharpDrop)
    {
        paragraphs << QCoreApplication::translate("QCDiagnoseTextBuilder", "The maximum 5-minute average SNR drop is %1 dB-Hz.")
            .arg(static_cast<double>(m.dropWindows.first().avgCN0Drop), 0, 'f', 1);
    }

    return paragraphs;
}

QStringList QCDiagnoseTextBuilder::buildMpSection(const DiagnosticInput &input,
                                                  const DiagnosticResult &result)
{
    Q_UNUSED(result);
    QStringList paragraphs;
    const MpMetrics &m = input.mp;

    // Worst cross-frequency ratios (already computed by engine, but cheap to redo).
    double worst03 = -1.0, worst06 = -1.0, worst09 = -1.0;
    for (int freq = 0; freq < QC_NUM_FREQ; ++freq)
    {
        float v03 = m.groupRatioAve[freq][0];
        float v06 = m.groupRatioAve[freq][1];
        float v09 = m.groupRatioAve[freq][2];
        if (v03 < 0.0f || v06 < 0.0f || v09 < 0.0f)
        {
            continue;
        }
        if (worst03 < 0.0 || v03 < worst03) worst03 = v03;
        if (worst06 < 0.0 || v06 < worst06) worst06 = v06;
        if (worst09 < 0.0 || v09 < worst09) worst09 = v09;
    }

    switch (result.mpLevel)
    {
        case MpLevel::Level1:
        {
            paragraphs << QCoreApplication::translate("QCDiagnoseTextBuilder",
                "Multipath is overall stable. Most observations remain at low "
                "multipath levels: the <0.3 m ratio exceeds 60% (%1), the "
                "<0.6 m ratio exceeds 90% (%2), and the <0.9 m ratio exceeds "
                "97% (%3). Impact on pseudorange and positioning accuracy is "
                "limited.")
                .arg(QString::number(worst03 * 100.0, 'f', 1) + '%')
                .arg(QString::number(worst06 * 100.0, 'f', 1) + '%')
                .arg(QString::number(worst09 * 100.0, 'f', 1) + '%');
            break;
        }
        case MpLevel::Level2:
        {
            QString sysName = (m.worstSysIndex >= 0)
                ? systemShortName(m.worstSysIndex)
                : QCoreApplication::translate("QCDiagnoseTextBuilder", "N/A");
            paragraphs << QCoreApplication::translate("QCDiagnoseTextBuilder",
                "Multipath impact is noticeable. The <0.3 m ratio is %1 "
                "(between 40% and 60%), indicating moderate reflectors in the "
                "observation environment that may affect code smoothing, "
                "convergence speed, and positioning stability. The worst-performing "
                "system is %2.")
                .arg(QString::number(worst03 * 100.0, 'f', 1) + '%')
                .arg(sysName);
            break;
        }
        case MpLevel::Level3:
        {
            paragraphs << QCoreApplication::translate("QCDiagnoseTextBuilder",
                "Multipath is severe; the site environment requires immediate "
                "attention. Multiple systems and frequencies show elevated multipath "
                "with prolonged anomaly periods. Direct use in high-precision "
                "analysis is not recommended; optimize the site environment or "
                "filter anomaly intervals.");
            break;
        }
    }

    return paragraphs;
}

QStringList QCDiagnoseTextBuilder::buildAvailabilitySection(const DiagnosticInput &input,
                                                            const DiagnosticResult &result,
                                                            const QCResultManager &mgr)
{
    Q_UNUSED(mgr);
    QStringList paragraphs;
    const AvailabilityMetrics &m = input.avail;

    switch (result.availLevel)
    {
        case AvailLevel::Level1:
        {
            paragraphs << QCoreApplication::translate("QCDiagnoseTextBuilder",
                "Observation data is continuous and normal. Per-system use rates: %1.")
                .arg(formatUseRateList(m));
            break;
        }
        case AvailLevel::Level2:
        {
            paragraphs << QCoreApplication::translate("QCDiagnoseTextBuilder",
                "Use rate for system %1 is %2, notably below the peer average "
                "(%3).")
                .arg(systemShortName(result.availOutlierSysIndex))
                .arg(QString::number(m.outlierSysUseRate * 100.0, 'f', 1) + '%')
                .arg(QString::number(m.peerAvgUseRate * 100.0, 'f', 1) + '%');
            break;
        }
        case AvailLevel::Level3:
        {
            const AvailabilityMetrics::BreakSummary &b = m.topBreaks.first();
            paragraphs << QCoreApplication::translate("QCDiagnoseTextBuilder",
                "A few brief observation breaks occurred. Total break count: %1; "
                "longest break: %2 minutes around %3. Please monitor equipment "
                "status.")
                .arg(m.obsBreakCount)
                .arg(b.durationSec / 60.0, 0, 'f', 1)
                .arg(b.startTime.toString("yyyy/MM/dd HH:mm"));
            break;
        }
        case AvailLevel::Level4:
        {
            const AvailabilityMetrics::BreakSummary &b = m.topBreaks.first();
            paragraphs << QCoreApplication::translate("QCDiagnoseTextBuilder",
                "Prolonged observation breaks occurred. Total break count: %1; "
                "longest break: %2 minutes around %3. Please inspect equipment "
                "status.")
                .arg(m.obsBreakCount)
                .arg(b.durationSec / 60.0, 0, 'f', 1)
                .arg(b.startTime.toString("yyyy/MM/dd HH:mm"));
            break;
        }
    }

    // FR-09: append an anomaly remark for systems that were observed but
    // report zero use rate (e.g. BDS with ephemeris issues). Applies to
    // every level so the cause is surfaced regardless of the level verdict.
    // The anomaly predicate is shared with the KPI availability card via
    // anomalousSystemNames() (single source of truth, FR-A3).
    const QStringList anomalousSys = anomalousSystemNames(m);
    if (!anomalousSys.isEmpty())
    {
        paragraphs << QCoreApplication::translate("QCDiagnoseTextBuilder",
            "System(s) %1 have no availability despite being observed. "
            "Suspected ephemeris/data anomaly; please verify.")
            .arg(anomalousSys.join(", "));
    }

    return paragraphs;
}

QString QCDiagnoseTextBuilder::formatUseRateList(const AvailabilityMetrics &avail)
{
    QStringList parts;
    for (int sys = 0; sys < QC_MAX_NUM_SYS; ++sys)
    {
        if (avail.useRateSys[sys] > 0.0)
        {
            parts << QString("%1 %2")
                      .arg(systemShortName(sys))
                      .arg(QString::number(avail.useRateSys[sys] * 100.0, 'f', 1) + '%');
        }
    }
    if (parts.isEmpty())
    {
        return QCoreApplication::translate("QCDiagnoseTextBuilder", "N/A");
    }
    return parts.join(", ");
}

QStringList QCDiagnoseTextBuilder::anomalousSystemNames(const AvailabilityMetrics &avail)
{
    // FR-09 / FR-A3: single source of truth for the "observed but zero use
    // rate" anomaly predicate. Consumed by buildAvailabilitySection() (detail
    // remark) and availKeyMetrics() (KPI card hint) so the two views never
    // drift apart; only the formatting differs at each call site.
    QStringList names;
    for (int sys = 0; sys < QC_MAX_NUM_SYS; ++sys)
    {
        if (avail.sysObserved[sys] && avail.useRateSys[sys] <= 0.0)
        {
            names << systemShortName(sys);
        }
    }
    return names;
}

QString QCDiagnoseTextBuilder::systemShortName(int sysIndex)
{
    switch (sysIndex)
    {
        case 0: return QStringLiteral("GPS");
        case 1: return QStringLiteral("SBAS");
        case 2: return QStringLiteral("GLO");
        case 3: return QStringLiteral("GAL");
        case 4: return QStringLiteral("QZSS");
        case 5: return QStringLiteral("BDS");
        case 6: return QStringLiteral("XW1");
        default: return QCoreApplication::translate("QCDiagnoseTextBuilder", "N/A");
    }
}

QString QCDiagnoseTextBuilder::frequencyLabel(int freqIndex)
{
    return QCoreApplication::translate("QCDiagnoseTextBuilder", "Freq #%1").arg(freqIndex + 1);
}

} // namespace QCDiag
