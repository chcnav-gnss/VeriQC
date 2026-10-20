/**********************************************************************//**
 * 		VeriQC
 *
 * 		UI Diagnostic Report Exporter Module
 * *-
 * @file QCDiagnoseReportExporter.h
 * @author CHC
 * @date 2026-06-15
 * @brief Renders diagnostic summaries to a standalone HTML file.
 *
 * For each QC object id supplied, the exporter:
 *   1. Pulls QC_RESULT_T via QCObjectManager.
 *   2. Builds DiagnosticInput (QCDiagnoseData).
 *   3. Runs DiagnoseEngine to obtain a DiagnosticResult.
 *   4. Composes paragraphs via QCDiagnoseTextBuilder.
 *   5. Writes a self-contained HTML report.
 *
 * Output file naming: <basename>_QCDiag.html next to <basename>_QC.html.
 *
 **************************************************************************/
#ifndef QCDIAGNOSEREPORTEXPORTER_H
#define QCDIAGNOSEREPORTEXPORTER_H

#include <QString>
#include <QVector>

/**
 * @class QCDiagnoseReportExporter
 * @brief Static facade for rendering the diagnostic summary HTML report.
 */
class QCDiagnoseReportExporter
{
public:
    /**
     * @brief Export diagnostic reports for a batch of QC objects.
     *
     * Behavior depends on the @p target count:
     * - When @p qcObjectIDs contains a single id, treats @p filePath as
     *   the exact output file path (overwrites if exists).
     * - When @p qcObjectIDs contains multiple ids, treats @p filePath as
     *   a directory; each report is written as
     *   <dir>/<basename>_QCDiag.html.
     *
     * Objects whose QC status is QCStatusNotExecuted are silently
     * skipped.
     *
     * @param[in] filePath   Output file (single) or directory (batch).
     * @param[in] qcObjectIDs IDs of QC objects to export.
     * @return Number of reports actually written; 0 on hard failure.
     */
    static int exportToHtml(const QString &filePath,
                            const QVector<int> &qcObjectIDs);

private:
    /**
     * @brief Render one QC object to HTML and write it to @p outputPath.
     * @return True on success.
     */
    static bool writeOneHtml(const QString &outputPath, int qcObjectID);

    /**
     * @brief Compose the full HTML document for one QC object.
     *
     * Exposed for testing; not part of the public surface.
     */
    static QString composeHtml(int qcObjectID);
};

#endif // QCDIAGNOSEREPORTEXPORTER_H
