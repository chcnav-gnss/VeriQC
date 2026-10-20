/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI QC Object Module
 * *-
 * @file QCObject.h
 * @author CHC
 * @date 2025-09-28
 * @brief QC Object for one OBS file
 * 
**************************************************************************/
#ifndef QCOBJECT_H
#define QCOBJECT_H

#include <QSharedPointer>
#include "QCConfigurator.h"
#include "QCProcessor.h"
#include "QCResultManager.h"


class QCObject
{
public:
    enum QCResultStatusEnum
    {
        QCStatusNotExecuted,    // qc not executed
        QCStatusPassed,         // qc executed, and qc check pass
        QCStatusFailed,         // qc executed, and qc check fail
    };
    struct QCCheckStatusInfoStruct
    {
        bool useRatePass;
        bool cycleJumpRatioPass;
        bool mpPass[UI_COMPLEX_MP_MAX_NUM];
        bool snrPass[UI_COMPLEX_MP_MAX_NUM];
    };

public:
    explicit QCObject(int id);
    QString getName()const;
    int getID()const;
    inline const QSharedPointer<QCConfigurator> & getQCConfigurator()const {return m_configurator;}
    inline const QSharedPointer<QCResultManager> & getQCResultManager()const {return m_resultManager;}
    bool doQC(QCProcessCallbackFun callback, bool &cancelQC);
    bool doRinexConvert(QCProcessCallbackFun callback, bool &cancel);
    const QString &getLastErrorMsg() const;
    /** Last doQC() processor return value: distinguishes execution failure
     *  from QCStatusFailed's threshold-based quality verdict. */
    bool isQCRunSucceeded() const;
    /** Empty when fine; "No valid data: ..." (run counts as failed) or
     *  "Note: ..." (degraded run, results still valid). */
    const QString &getDataExceptionMsg() const;
    /** Convert-mode results (OutDir snapshot diff of the last run). */
    const QStringList &getConvertOutputFiles() const;
    bool isConvertSucceeded() const;
    QCResultStatusEnum getQCStatus() const;
    const QCCheckStatusInfoStruct &getQCCheckStatusInfo()const;
    void changeInputFormat(QCConfigurator::RawFileDataFormatEnum format);
protected:
    void updateQCStatus();
private:
    int m_id = 0;
    QSharedPointer<QCConfigurator> m_configurator;
    QSharedPointer<QCResultManager> m_resultManager;
    QSharedPointer<QCProcessor> m_processor;
    QCResultStatusEnum m_qcStatus = QCResultStatusEnum::QCStatusNotExecuted;
    /** Zero-initialized: exportToExcel() reads the pass flags on the FAILED
     *  branch only after updateQCStatus() filled them, but a default value
     *  keeps that safety independent of branch order. */
    QCCheckStatusInfoStruct m_qcCheckStatusInfo = {};
    bool m_lastRunOK = false;
};

#endif // QCOBJECT_H
