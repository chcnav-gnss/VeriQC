/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI QC Object Module
 * *-
 * @file QCProcessor.h
 * @author CHC
 * @date 2025-09-28
 * @brief processor of QC Object
 * 
**************************************************************************/
#ifndef QCPROCESSOR_H
#define QCPROCESSOR_H

#include <QSharedPointer>
#include <QStringList>
#include <functional>
#include "QCConfigurator.h"
#include "QCResultManager.h"

using QCProcessCallbackFun = std::function<void(QString/* name*/,int/* progressRate*/)>;

class QCObject;
class QCProcessor
{
public:
    QCProcessor(const QSharedPointer<QCConfigurator> & configurator, const QSharedPointer<QCResultManager> &resultManager);
    bool doQC(const QString &name, QCProcessCallbackFun callback, bool &cancelQC);
    bool doRinexConvert(const QString &name, QCProcessCallbackFun callback, bool &cancel);
    const QString &getLastErrorMsg()const;
    /** Last QC run data-validity note: empty when fine; "No valid data: ..."
     *  for severe engine exception codes / no epochs (run counts as failed);
     *  "Note: ..." for degraded runs (missing nav / SPP / partial systems). */
    const QString &getDataExceptionMsg()const;
    /** Whether @p msg is a "No valid data" note (severe engine exception /
     *  no epochs) — such runs count as failed for the batch exit code and
     *  surface as a distinct status in the Excel. Single-source counterpart
     *  of the prefix built inside doQC(), so wording changes cannot desync
     *  the two sides. */
    static bool isNoValidDataMsg(const QString &msg);
    /** Output files produced by the last doRinexConvert() (OutDir snapshot
     *  diff); isConvertSucceeded() is false when the list is empty. */
    const QStringList &getConvertOutputFiles()const;
    bool isConvertSucceeded()const;
private:
    QSharedPointer<QCConfigurator> m_configurator;
    QSharedPointer<QCResultManager> m_resultManager;
    QString m_lastErrorMsg;
    QString m_dataExceptionMsg;
    QStringList m_convertOutputFiles;
    bool m_convertSucceeded = false;
};

#endif // QCPROCESSOR_H
