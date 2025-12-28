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
private:
    QSharedPointer<QCConfigurator> m_configurator;
    QSharedPointer<QCResultManager> m_resultManager;
    QString m_lastErrorMsg;
};

#endif // QCPROCESSOR_H
