/**********************************************************************//**
 * 		VeriQC
 *
 * 		UI Diagnostic Engine Module
 * *-
 * @file QCDiagnoseEngine.h
 * @author CHC
 * @date 2026-06-15
 * @brief Level-mapping engine for the four diagnostic dimensions.
 *
 * The engine is stateless and uses the "first-match-wins" approach over
 * the threshold tables defined in QCDiagnoseConstants. Each dimension
 * (cycle slip / SNR / multipath / availability) returns a Level enum
 * and decorates the DiagnosticResult with auxiliary flags that the text
 * builder uses to emit optional remarks.
 *
 **************************************************************************/
#ifndef QCDIAGNOSEENGINE_H
#define QCDIAGNOSEENGINE_H

#include "QCDiagnoseData.h"

namespace QCDiag
{

/**
 * @class DiagnoseEngine
 * @brief Stateless helper that turns DiagnosticInput into DiagnosticResult.
 *
 * All members are static; the class is not meant to be instantiated.
 */
class DiagnoseEngine
{
public:
    /**
     * @brief Top-level entry: classify every dimension and fill the
     *        auxiliary flags in @p out.
     *
     * @param[in]     input  Derived metrics for one file.
     * @param[in,out] out    Result buffer; levels and flags are overwritten.
     */
    static void diagnose(const DiagnosticInput &input, DiagnosticResult &out);

    /**
     * @brief Classify the cycle-slip dimension and decorate aux flags.
     * @return Level in [Level1 .. Level4].
     */
    static SlipLevel diagnoseSlip(const SlipMetrics &m, DiagnosticResult &out);

    /**
     * @brief Classify the SNR dimension and decorate aux flags.
     * @return Level in [Level1 .. Level4].
     */
    static SnrLevel diagnoseSnr(const SnrMetrics &m, DiagnosticResult &out);

    /**
     * @brief Classify the multipath dimension and decorate aux flags.
     * @return Level in [Level1 .. Level3].
     */
    static MpLevel diagnoseMp(const MpMetrics &m, DiagnosticResult &out);

    /**
     * @brief Classify the availability dimension and decorate aux flags.
     * @return Level in [Level1 .. Level4].
     */
    static AvailLevel diagnoseAvailability(const AvailabilityMetrics &m, DiagnosticResult &out);

private:
    /** Fill slip auxiliary flags (severe satellite, system outlier). */
    static void decorateSlipAux(const SlipMetrics &m, DiagnosticResult &out);

    /** Fill SNR auxiliary flags (abnormal frequency). */
    static void decorateSnrAux(const SnrMetrics &m, DiagnosticResult &out);

    /** Fill multipath auxiliary flags (worst system). */
    static void decorateMpAux(const MpMetrics &m, DiagnosticResult &out);

    /** Fill availability auxiliary flags (outlier system, breaks). */
    static void decorateAvailabilityAux(const AvailabilityMetrics &m, DiagnosticResult &out);
};

} // namespace QCDiag

#endif // QCDIAGNOSEENGINE_H
