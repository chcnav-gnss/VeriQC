#include "RinexMergeProcessor.h"
#include <QDir>
#include "VeriQC.h"
#include "PathUtils.h"

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
    /** Encode each input path individually (short-path on Windows) then join
     *  with ASCII comma; the C layer splits RawFile on ',' in multi-file mode. */
    QByteArray inputFileData;
    inputFileData.reserve(m_inputFilePaths.size() * 64);
    for (int i = 0; i < m_inputFilePaths.size(); ++i)
    {
        if (i > 0)
        {
            inputFileData.append(',');
        }
        inputFileData.append(PathUtils::toLocalFilePathBytes(m_inputFilePaths.at(i)));
    }
    Q_ASSERT(inputFileData.size() < MAX_FILE_NAME_LENGTH - 1);
    Q_ASSERT(m_inputFilePaths.size() <= 10);
    memcpy(config.RawFile, inputFileData.constData(), inputFileData.size());
    /** toLocalDirPathBytes normalizes the trailing separator, creates the
     *  directory, and encodes the path for the C layer's fopen(). */
    QByteArray outDirBytes = PathUtils::toLocalDirPathBytes(m_outputFilePath);
    memcpy(config.OutDir, outDirBytes.constData(), outDirBytes.size());

    RinexProcesser(&config);
}

void RinexMergeProcessor::onUpdateRate()
{
    int rate = GetRinexProgress();
    emit this->rateChanged(rate);
}
