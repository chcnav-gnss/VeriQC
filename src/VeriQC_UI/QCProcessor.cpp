#include "QCProcessor.h"
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>
#include "AppSettingsManagement.h"

namespace
{
/** "No valid data" prefix shared by the messages built in doQC() and by
 *  isNoValidDataMsg() consumers (batch exit code, Excel status); single-sourced
 *  so a wording change cannot silently flip a failed run to a successful one. */
const char NoValidDataPrefix[] = "No valid data";

/** Engine exception codes split into file-level severe failures (no usable
 *  data - the run must count as failed) and degraded runs (missing nav / SPP
 *  / partial systems - results are still valid, note only). */
QString qcExceptionCodeText(int code)
{
    switch (code)
    {
    case QC_EXCEPTION_UNSUPPORTED_FORMAT: return QStringLiteral("unsupported data format");
    case QC_EXCEPTION_OBS_FILE_OPEN_FAILED: return QStringLiteral("observation file open failed");
    case QC_EXCEPTION_NAV_FILE_MISSING: return QStringLiteral("navigation file missing");
    case QC_EXCEPTION_PARSE_HEADER_FAILED: return QStringLiteral("file header parse failed");
    case QC_EXCEPTION_PARSE_EPOCH_FAILED: return QStringLiteral("epoch parse failed");
    case QC_EXCEPTION_PARTIAL_SYS_FAILED: return QStringLiteral("some systems failed to process");
    case QC_EXCEPTION_SPP_ALL_FAILED: return QStringLiteral("all SPP solutions failed");
    case QC_EXCEPTION_SYS_NAV_UNMATCHED: return QStringLiteral("observation without matched navigation for a system");
    case QC_EXCEPTION_SYS_SPP_UNSOLVED: return QStringLiteral("system SPP unsolved");
    default: return QStringLiteral("exception code %1").arg(code);
    }
}

bool isSevereQCException(int code)
{
    return QC_EXCEPTION_UNSUPPORTED_FORMAT == code
        || QC_EXCEPTION_OBS_FILE_OPEN_FAILED == code
        || QC_EXCEPTION_PARSE_HEADER_FAILED == code
        || QC_EXCEPTION_PARSE_EPOCH_FAILED == code;
}

bool isDegradedQCException(int code)
{
    return QC_EXCEPTION_NAV_FILE_MISSING == code
        || QC_EXCEPTION_PARTIAL_SYS_FAILED == code
        || QC_EXCEPTION_SPP_ALL_FAILED == code
        || QC_EXCEPTION_SYS_NAV_UNMATCHED == code
        || QC_EXCEPTION_SYS_SPP_UNSOLVED == code;
}

/** RINEX observation product names emitted by the converter: <base>.<YY>o
 *  (e.g. station.26o, case-insensitive) or <base>.rnx. Nav/meta outputs
 *  (.26n/.26p/...), *_BaseObsSatCount.dat and *_DebugInfo.dat do not match. */
const QRegularExpression &obsOutputNameRegex()
{
    static const QRegularExpression regex(
        QStringLiteral("\\.(rnx|[0-9]{2}[oOdD])$"),
        QRegularExpression::CaseInsensitiveOption);
    return regex;
}
}

QCProcessor::QCProcessor(const QSharedPointer<QCConfigurator> & configurator, const QSharedPointer<QCResultManager> &resultManager)
    :m_configurator(configurator),m_resultManager(resultManager)
{

}

bool QCProcessor::doQC(const QString &name, QCProcessCallbackFun callback, bool &cancelQC)
{
    int ret = -1;
    CHCQC_READER_T* qcReader = NULL;
#ifdef CONSOLE
    CHCQC_GNSSCFG_T config = m_configurator->getGNSSAPIConfig();
#else
    CHCQC_GNSSCFG_T config = m_configurator->generateAPIConfig();
#endif
    unsigned long long fileTotalSize = 0;
    unsigned long long fileCurrentSize = 0;
    int currentProgressRate = 0;

    callback(name,0);
    m_lastErrorMsg.clear();
    m_dataExceptionMsg.clear();
    /** init */
    qcReader = (CHCQC_READER_T*)VERIQC_MALLOC(sizeof(CHCQC_READER_T));
    if (!qcReader)
    {
        m_lastErrorMsg = "<GNSSQC error>: Failed to allocate QC reader.";
        return false;
    }
    memset(qcReader,0,sizeof(CHCQC_READER_T));
    /** Output dir is created upstream by PathUtils::toLocalDirPathBytes
     *  (inside generateAPIConfig for GUI mode, inside CommandLineProcessor
     *  for CONSOLE mode), so no mkpath is needed here. */
    ret = InitQCReader(qcReader);
    if(ret == -1)
    {
        m_lastErrorMsg = "<GNSSQC error>: Failed to init QC reader.";
        VERIQC_FREE(qcReader);
        return false;
    }
    InitRTKCore((double)config.EleDeg);
    BOOL state = SyncGNSSQCState(&config,qcReader);
    if(!state)
    {
        m_lastErrorMsg = "<GNSSQC error>: Failed to sync QC state (decoder/input/output files).";
        FreeQCReader(qcReader);
        return false;
    }
    GNSSPreProcess(&config, qcReader);
    UpdateQCOpt(qcReader->pOpt,&qcReader->QcRes);
    qcReader->IterNo = 0;
    fileTotalSize = qcReader->RawFileSize[0];
    m_resultManager->resetResult();    // clear old result data cache
    int CurrentEpochNum = 0;
    /** process */
    while (!cancelQC)
    {
        if((ret = QCProcessCore(&config,qcReader,qcReader->IterNo)) < 0)
        {
            break;
        }
        if(qcReader->QcRes.MaxEpochNum[0] > CurrentEpochNum)   // new epoch obs data
        {
            CurrentEpochNum = qcReader->QcRes.MaxEpochNum[0];
            /** cache epoch result data */
            QCResultManager::QCEpochResult epochResult;
            epochResult.epochObs.time = qcReader->pObs[0][0].Time;
            epochResult.epochObs.StaPos[0] = qcReader->pObs[0][0].StaPos[0];
            epochResult.epochObs.StaPos[1] = qcReader->pObs[0][0].StaPos[1];
            epochResult.epochObs.StaPos[2] = qcReader->pObs[0][0].StaPos[2];
            for (int i = 0; i < qcReader->pObs[0][0].ObsNum && i < QC_MAX_NUM_OBS; ++i)
            {
                QCResultManager::QCEpochSatObsData epochSatData = {};
                epochSatData.sat = qcReader->pObs[0][0].Data[i].Sat;
                epochSatData.sys = qcReader->pObs[0][0].Data[i].Sys;
                epochSatData.prn = qcReader->pObs[0][0].Data[i].PRN;
                epochSatData.nfre = qcReader->pObs[0][0].Data[i].NumFre;
                epochSatData.frq = qcReader->pObs[0][0].Data[i].GLOFreq;
                memcpy(epochSatData.SNR,qcReader->pObs[0][0].Data[i].SNR,sizeof(epochSatData.SNR));
                memcpy(epochSatData.LLI,qcReader->pObs[0][0].Data[i].LLI,sizeof(epochSatData.LLI));
                memcpy(epochSatData.L,qcReader->pObs[0][0].Data[i].L,sizeof(epochSatData.L));
                memcpy(epochSatData.P,qcReader->pObs[0][0].Data[i].P,sizeof(epochSatData.P));
                memcpy(epochSatData.D,qcReader->pObs[0][0].Data[i].D,sizeof(epochSatData.D));
                memcpy(epochSatData.azel,qcReader->pObs[0][0].Data[i].Azel,sizeof(epochSatData.azel));
                memcpy(epochSatData.slipFlag,qcReader->QcRes.SlipFlag[0][epochSatData.sat-1],sizeof(int)*QC_NUM_FREQ);
                memcpy(epochSatData.curMP,qcReader->QcRes.CurMP[0][epochSatData.sat-1],sizeof(double)*QC_NUM_FREQ);

                epochResult.epochObs.data.append(epochSatData);
            }
            epochResult.epochSolInfo = qcReader->EpochSolInfo;
            m_resultManager->addEpochResult(epochResult);
        }

        /** update progress rate */
        fileCurrentSize = qcReader->CurFilePos[0];
        int progress = fileTotalSize > 0 ? (100 * fileCurrentSize / fileTotalSize) : 100;
        if(progress != currentProgressRate)
        {
            currentProgressRate = progress;
            callback(name,currentProgressRate);
        }
    }
    if(cancelQC)
    {
        /** free */
        EndRTKCore();
        FreeQCReader(qcReader);
        return false;
    }
    /** analysis */
    GetQCAvePos(&qcReader->QcRes);
    UpdateQCRes(&qcReader->QcRes,qcReader->pOpt);
    m_resultManager->updateQCResult(qcReader->QcRes);
    /** write report html */
    WriteQCStatisticsHtml(&qcReader->QcRes, qcReader->pOpt);
    /** get station attribute info */
    QCResultManager::StationAttributeInfo stationAttributeInfo;
    stationAttributeInfo.ecef[0] = qcReader->QcRes.PosXYZ[0][0];
    stationAttributeInfo.ecef[1] = qcReader->QcRes.PosXYZ[0][1];
    stationAttributeInfo.ecef[2] = qcReader->QcRes.PosXYZ[0][2];
    stationAttributeInfo.startObsTime = QCResultManager::qcTime2DateTime(qcReader->QcRes.StartTime[0]);
    stationAttributeInfo.endObsTime = QCResultManager::qcTime2DateTime(qcReader->QcRes.EndTime[0]);
    stationAttributeInfo.interval = qcReader->QcRes.Interval[0];
    stationAttributeInfo.EpochCount = m_resultManager->getEpochResultList().size();
    stationAttributeInfo.receiverManufacturer = qcReader->RinexObsHeaderInfo.Recfacturer;
    stationAttributeInfo.receiverType = qcReader->RinexObsHeaderInfo.RecType;
    stationAttributeInfo.receiverVersion = qcReader->RinexObsHeaderInfo.RecVer;
    stationAttributeInfo.receiverSN = qcReader->RinexObsHeaderInfo.RecSN;
    stationAttributeInfo.antennaManufacturer = qcReader->RinexObsHeaderInfo.AntManufacturer;
    stationAttributeInfo.antennaType = qcReader->RinexObsHeaderInfo.AntType;
    stationAttributeInfo.antennaSN = qcReader->RinexObsHeaderInfo.AntSN;
    stationAttributeInfo.AntDeltaH = qcReader->RinexObsHeaderInfo.AntDeltaH;
    m_resultManager->updateStationAttributeInfo(stationAttributeInfo);
    /** data validity: rover-channel engine exception code + epoch check.
     *  Severe codes / no epochs => the run produced no usable data (batch
     *  exit code must treat it as failed); degraded codes stay a note only. */
    const int roverExceptionCode = qcReader->QcRes.FileException[0].Code;
    if (isSevereQCException(roverExceptionCode))
    {
        m_dataExceptionMsg = QStringLiteral("%1: %2").arg(QLatin1String(NoValidDataPrefix), qcExceptionCodeText(roverExceptionCode));
    }
    else if (m_resultManager->getEpochResultList().isEmpty())
    {
        m_dataExceptionMsg = QStringLiteral("%1: no epoch was processed").arg(QLatin1String(NoValidDataPrefix));
    }
    else if (QC_EXCEPTION_OK != roverExceptionCode && isDegradedQCException(roverExceptionCode))
    {
        m_dataExceptionMsg = QStringLiteral("Note: %1").arg(qcExceptionCodeText(roverExceptionCode));
    }
    /** free */
    EndRTKCore();
    FreeQCReader(qcReader);
    callback(name,100);
    return true;
}

bool QCProcessor::doRinexConvert(const QString &name, QCProcessCallbackFun callback, bool &cancel)
{
    Q_UNUSED(cancel)    //@TODO
#ifdef CONSOLE
    CHCQC_RINEXCFG_T rinexConfig = m_configurator->getRinexConvertAPIConfig();
    const QString inputFilePath = QString::fromLocal8Bit(rinexConfig.RawFile);
#else
    const QCConfigurator::RinexConvertSettings &rinexConvertSettings = AppSettingsManagement::getInstance()->getRinexConvertSettings();
    /** convert to QC API rinex convert config */
    CHCQC_RINEXCFG_T rinexConfig = m_configurator->generateAPIRinexConvertConfig(rinexConvertSettings);
    const QString inputFilePath = m_configurator->getRawFileInfo().rawFilePath;
#endif

    callback(name,0);
    /** RinexProcesser() is void and prints errors to the console only, so the
     *  output-dir snapshot diff (new files, plus same-base files touched
     *  during the run) is the only per-file output evidence; success further
     *  requires a RINEX observation product among the diff (obsOutputNameRegex). */
    const QString outDirPath = QString::fromLocal8Bit(rinexConfig.OutDir);
    auto snapshotOutDir = [&outDirPath]() {
        QHash<QString, QDateTime> files;
        for (const QFileInfo &info : QDir(outDirPath).entryInfoList(QDir::Files))
        {
            files.insert(info.fileName(), info.lastModified());
        }
        return files;
    };
    const QHash<QString, QDateTime> outDirBefore = snapshotOutDir();

    RinexProcesser(&rinexConfig);

    const QHash<QString, QDateTime> outDirAfter = snapshotOutDir();
    const QString inputBase = QFileInfo(inputFilePath).completeBaseName();
    m_convertOutputFiles.clear();
    for (auto it = outDirAfter.constBegin(); it != outDirAfter.constEnd(); ++it)
    {
        //single constFind() lookup instead of contains() + value() per file
        const auto beforeIt = outDirBefore.constFind(it.key());
        const bool isNewFile = outDirBefore.constEnd() == beforeIt;
        const bool sameBaseUpdated = outDirBefore.constEnd() != beforeIt
            && beforeIt.value() != it.value()
            && QFileInfo(it.key()).completeBaseName().startsWith(inputBase);
        if (isNewFile || sameBaseUpdated)
        {
            m_convertOutputFiles.append(it.key());
        }
    }
    m_convertOutputFiles.sort();
    /** The engine returns normally even when the forced input DataType decodes
     *  nothing (a wrong -c DataType still emits nav stubs like <base>.26p plus
     *  *_DebugInfo.dat side artifacts), so "diff non-empty" alone would mark
     *  such runs as converted. Require an actual RINEX observation product. */
    m_convertSucceeded = false;
    for (const QString &outputName : m_convertOutputFiles)
    {
        if (outputName.contains(obsOutputNameRegex()))
        {
            m_convertSucceeded = true;
            break;
        }
    }
    callback(name,100);
    return true;
}

const QString &QCProcessor::getLastErrorMsg() const
{
    return m_lastErrorMsg;
}

const QString &QCProcessor::getDataExceptionMsg() const
{
    return m_dataExceptionMsg;
}

bool QCProcessor::isNoValidDataMsg(const QString &msg)
{
    return msg.startsWith(QLatin1String(NoValidDataPrefix));
}

const QStringList &QCProcessor::getConvertOutputFiles() const
{
    return m_convertOutputFiles;
}

bool QCProcessor::isConvertSucceeded() const
{
    return m_convertSucceeded;
}
