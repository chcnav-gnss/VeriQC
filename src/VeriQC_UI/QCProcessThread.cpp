#include "QCProcessThread.h"


QCProcessThread::QCProcessThread(QObject *parent)
    :QThread(parent)
{
    m_rinexConvertRateTimer = new QTimer(this);
    m_rinexConvertRateTimer->setInterval(500);
    connect(m_rinexConvertRateTimer,&QTimer::timeout,this,&QCProcessThread::onUpdateRinexConvertRate);
}

void QCProcessThread::clearQCObjects()
{
    m_qcObjects.clear();
}

void QCProcessThread::setQCObjects(const QVector<QSharedPointer<QCObject> > &qcObjects)
{
    m_qcObjects = qcObjects;
}

void QCProcessThread::setProcessMode(ProcessModeEnum mode)
{
    m_processMode = mode;
}

void QCProcessThread::cancel()
{
    m_cancel = true;
}

void QCProcessThread::run()
{
    m_cancel = false;
    int objectCount = m_qcObjects.size();
    for (int i = 0; i < objectCount && !m_cancel; ++i)
    {
        m_currentQCObjectIndex.store(i);
        int startRate = i*100/objectCount;
        QCProcessCallbackFun callbackFun = [this,startRate,objectCount](QString name, int rate){
            emit this->qcProgressRateChanged(startRate + rate/objectCount,name,rate);
        };
        bool ret = false;
        if(QCProcessThread::RinexConvertMode == m_processMode)
        {
            QMetaObject::invokeMethod(this,"startRinexConvertRateTimer",Qt::QueuedConnection);
            ret = m_qcObjects[i]->doRinexConvert(callbackFun,m_cancel);
            QMetaObject::invokeMethod(this,"stopRinexConvertRateTimer",Qt::QueuedConnection);
        }
        else
        {
            ret = m_qcObjects[i]->doQC(callbackFun,m_cancel);
        }
        if(!ret)
        {
            if(!m_cancel)
            {
                emit this->qcProcessError(m_qcObjects[i]->getName(), m_qcObjects[i]->getLastErrorMsg());
            }
        }
    }
    m_cancel = false;
}

void QCProcessThread::onUpdateRinexConvertRate()
{
    int index = m_currentQCObjectIndex.load();
    int objectCount = m_qcObjects.size();
    QString name = m_qcObjects[index]->getName();
    int startRate = index*100/objectCount;
    int rate = GetRinexProgress();
    emit this->qcProgressRateChanged(startRate + rate/objectCount,name,rate);
}

void QCProcessThread::startRinexConvertRateTimer()
{
    m_rinexConvertRateTimer->start();
}

void QCProcessThread::stopRinexConvertRateTimer()
{
    m_rinexConvertRateTimer->stop();
}
