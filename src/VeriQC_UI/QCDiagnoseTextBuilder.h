/**********************************************************************//**
 * 		VeriQC
 *
 * 		UI Diagnostic Text Builder Module
 * *-
 * @file QCDiagnoseTextBuilder.h
 * @author CHC
 * @date 2026-06-15
 * @brief Builds localized paragraphs from a DiagnosticResult.
 *
 * Template strings live in the .cpp and are wrapped with tr() so that
 * Qt Linguist can extract them. Source text is English; Chinese
 * translation is maintained in VeriQC_UI_zh_CN.ts. Literal percent
 * characters are escaped as "%%" because QString::arg consumes "%N"
 * sequences as placeholders.
 *
 **************************************************************************/
#ifndef QCDIAGNOSETEXTBUILDER_H
#define QCDIAGNOSETEXTBUILDER_H

#include <QString>
#include <QStringList>
#include <QCoreApplication>
#include "QCDiagnoseData.h"

class QCResultManager;

namespace QCDiag
{

/**
 * @class QCDiagnoseTextBuilder
 * @brief Stateless helper that converts a DiagnosticResult into
 *        human-readable paragraphs.
 *
 * The class only contains static builders; Q_DECLARE_TR_FUNCTIONS
 * enables tr() in this non-QObject context.
 */
class QCDiagnoseTextBuilder
{
    Q_DECLARE_TR_FUNCTIONS(QCDiagnoseTextBuilder)

public:
    /**
     * @brief Compose the full diagnostic text for one file.
     *
     * The result is a multi-line string with section headers; suitable
     * for inline HTML rendering after light escaping.
     *
     * @param[in] input  Derived metrics (for filling numeric placeholders).
     * @param[in] result Diagnostic outcome (level + aux flags).
     * @param[in] mgr    Result manager (for satellite-name lookup).
     * @return Multi-line localized text.
     */
    static QString buildFullText(const DiagnosticInput &input,
                                 const DiagnosticResult &result,
                                 const QCResultManager &mgr);

    /**
     * @brief Build the cycle-slip section paragraphs.
     * @return One QString per paragraph.
     */
    static QStringList buildSlipSection(const DiagnosticInput &input,
                                        const DiagnosticResult &result,
                                        const QCResultManager &mgr);

    /**
     * @brief Build the SNR section paragraphs.
     */
    static QStringList buildSnrSection(const DiagnosticInput &input,
                                       const DiagnosticResult &result);

    /**
     * @brief Build the multipath section paragraphs.
     */
    static QStringList buildMpSection(const DiagnosticInput &input,
                                      const DiagnosticResult &result);

    /**
     * @brief Build the availability section paragraphs.
     */
    static QStringList buildAvailabilitySection(const DiagnosticInput &input,
                                                const DiagnosticResult &result,
                                                const QCResultManager &mgr);

    /**
     * @brief Format a use-rate list "GPS 95.0%, BDS 92.3%, ..." for the
     *        availability L1 template.
     */
    static QString formatUseRateList(const AvailabilityMetrics &avail);

    /**
     * @brief Collect short names of systems observed but with zero use rate
     *        (FR-09 / FR-A3 anomaly). Single source of truth for the anomaly
     *        predicate; consumed by both buildAvailabilitySection() (detail
     *        remark) and the KPI availability card (availKeyMetrics).
     * @return Short names (e.g. "BDS", "GAL"); empty when no anomaly.
     */
    static QStringList anomalousSystemNames(const AvailabilityMetrics &avail);

    /**
     * @brief Resolve a UI system index (0..5) to a localized system
     *        label (e.g. "GPS", "BDS").
     */
    static QString systemShortName(int sysIndex);

    /**
     * @brief Resolve a UI frequency index to a human-readable label.
     *
     * Frequency indexing follows the VeriQC layout (see VeriQC.h). This
     * helper returns a generic label such as "Freq #N" when the index
     * is not in a known system context.
     */
    static QString frequencyLabel(int freqIndex);
};

} // namespace QCDiag

#endif // QCDIAGNOSETEXTBUILDER_H
