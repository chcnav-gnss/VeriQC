/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI QC Object Module
 * *-
 * @file QCProcessThread.h
 * @author CHC
 * @date 2025-09-28
 * @brief process thread of QC Object Processor
 * 
**************************************************************************/
#ifndef QCPROCESSTHREAD_H
#define QCPROCESSTHREAD_H

#include <QThread>
#include <QVector>
#include <QTimer>
#include "QCObject.h"

class QCProcessThread : public QThread
{
    Q_OBJECT
public:
    enum ProcessModeEnum
    {
        QCMode,                 // Run QC
        RinexConvertMode       // Other format convert to RINEX
    };
public:
    QCProcessThread(QObject *parent = nullptr);
    void clearQCObjects();
    void setQCObjects(const QVector<QSharedPointer<QCObject>> &qcObjects);
    void setProcessMode(ProcessModeEnum mode);
    void cancel();
    Q_INVOKABLE void startRinexConvertRateTimer();
    Q_INVOKABLE void stopRinexConvertRateTimer();
signals:
    void qcProgressRateChanged(int totalRate, QString currentQCName, int currentQCRate);
    void qcProcessError(QString currentQCName, QString errorMsg);
protected:
    virtual void run() override;
    void onUpdateRinexConvertRate();

private:
    QVector<QSharedPointer<QCObject>> m_qcObjects;
    ProcessModeEnum m_processMode = QCProcessThread::QCMode;
    bool m_cancel = false;
    QTimer *m_rinexConvertRateTimer;
    QAtomicInt m_currentQCObjectIndex = 0;
};

#endif // QCPROCESSTHREAD_H
