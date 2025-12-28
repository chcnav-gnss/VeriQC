#include "RinexMergeProcessor.h"
#include <QDir>
#include "VeriQC.h"

RinexMergeProcessor::RinexMergeProcessor(QObject *parent) : QThread(parent)
{
    m_progressRateUpdateTimer = new QTimer(this);
    m_progressRateUpdateTimer->setInterval(500);

    connect(m_progressRateUpdateTimer,&QTimer::timeout,this,&RinexMergeProcessor::onUpdateRate);
    connect(this,&RinexMergeProcessor::finished,m_progressRateUpdateTimer,&QTimer::stop);
}

void RinexMergeProcessor::startProcess(const QStringList &inputFilePaths, const QString &outputDir)
{
    m_inputFilePaths = inputFilePaths;
    m_outputFilePath = outputDir;

    this->start();
    m_progressRateUpdateTimer->start();
}

void RinexMergeProcessor::run()
{
    CHCQC_RINEXCFG_T config = {};
    InitRinexCfg(&config);
    config.DataType = 11;
    config.OutputType = 0;
    QByteArray inputFileData = m_inputFilePaths.join(",").toLocal8Bit();
    Q_ASSERT(inputFileData.size() < MAX_FILE_NAME_LENGTH - 1);
    Q_ASSERT(m_inputFilePaths.size() <= 10);
    memcpy(config.RawFile,inputFileData.data(),inputFileData.size());
    QString outputDir = m_outputFilePath;
    if(!outputDir.endsWith("/") && !outputDir.endsWith("\\"))
    {
        outputDir += QDir::separator();
    }
    memcpy(config.OutDir,outputDir.toLocal8Bit().data(),outputDir.toLocal8Bit().size());

    RinexProcesser(&config);
}

void RinexMergeProcessor::onUpdateRate()
{
    int rate = GetRinexProgress();
    emit this->rateChanged(rate);
}
