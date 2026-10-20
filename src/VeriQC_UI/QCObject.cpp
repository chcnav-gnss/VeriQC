#include "QCObject.h"

QCObject::QCObject(int id)
    :m_id(id),m_configurator(new QCConfigurator()),m_resultManager(new QCResultManager()),m_processor(new QCProcessor(m_configurator,m_resultManager))
{

}

QString QCObject::getName() const
{
    return m_configurator->getFileName();
}

int QCObject::getID() const
{
    return m_id;
}

bool QCObject::doQC(QCProcessCallbackFun callback, bool &cancelQC)
{
    bool ret = m_processor->doQC(this->getName(), callback,cancelQC);
    m_lastRunOK = ret;
    if(ret)
    {
        this->updateQCStatus();
    }
    else
    {
        m_qcStatus = QCObject::QCStatusFailed;
    }
    return ret;
}

bool QCObject::doRinexConvert(QCProcessCallbackFun callback, bool &cancel)
{
    return m_processor->doRinexConvert(this->getName(), callback, cancel);
}

const QString &QCObject::getLastErrorMsg()const
{
    return m_processor->getLastErrorMsg();
}

bool QCObject::isQCRunSucceeded() const
{
    return m_lastRunOK;
}

const QString &QCObject::getDataExceptionMsg() const
{
    return m_processor->getDataExceptionMsg();
}

const QStringList &QCObject::getConvertOutputFiles() const
{
    return m_processor->getConvertOutputFiles();
}

bool QCObject::isConvertSucceeded() const
{
    return m_processor->isConvertSucceeded();
}

QCObject::QCResultStatusEnum QCObject::getQCStatus() const
{
    return m_qcStatus;
}

const QCObject::QCCheckStatusInfoStruct &QCObject::getQCCheckStatusInfo() const
{
    return m_qcCheckStatusInfo;
}

void QCObject::changeInputFormat(QCConfigurator::RawFileDataFormatEnum format)
{
    m_configurator->changeInputFormat(format);
    m_resultManager->resetResult();
    m_qcStatus = QCObject::QCStatusNotExecuted;
}

void QCObject::updateQCStatus()
{
    QCConfigurator::QCCheckConfigData checkConfig = this->getQCConfigurator()->getCheckConfig();
    QCResultManager::QCSysStatisticResultData result = this->getQCResultManager()->getSysStatisticResult(QCResultManager::QCSysTypeEnum::QCSysComplex);

    m_qcStatus = QCObject::QCStatusPassed;

    if(checkConfig.getTotalUseRate() > result.useRate)
    {
        m_qcStatus = QCObject::QCStatusFailed;
        m_qcCheckStatusInfo.useRatePass = false;
    }
    else
    {
        m_qcCheckStatusInfo.useRatePass = true;
    }
    if(checkConfig.CycleJumpRatio > result.cycleJumpRatio)
    {
        m_qcStatus = QCObject::QCStatusFailed;
        m_qcCheckStatusInfo.cycleJumpRatioPass = false;
    }
    else
    {
        m_qcCheckStatusInfo.cycleJumpRatioPass = true;
    }
    for (int i = 0; i < UI_COMPLEX_MP_MAX_NUM; ++i)
    {
        if(checkConfig.maxMP[i] < result.MP[i])
        {
            m_qcStatus = QCObject::QCStatusFailed;
            m_qcCheckStatusInfo.mpPass[i] = false;
        }
        else
        {
            m_qcCheckStatusInfo.mpPass[i] = true;
        }
    }
    for (int i = 0; i < UI_COMPLEX_MP_MAX_NUM; ++i)
    {
        if(checkConfig.minSNR[i] > result.SNR[i])
        {
            m_qcStatus = QCObject::QCStatusFailed;
            m_qcCheckStatusInfo.snrPass[i] = false;
        }
        else
        {
            m_qcCheckStatusInfo.snrPass[i] = true;
        }
    }

}
