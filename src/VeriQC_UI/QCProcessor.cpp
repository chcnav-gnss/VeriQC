#include "QCProcessor.h"
#include <QDir>
#include <QFileInfo>
#include "AppSettingsManagement.h"

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
    /** init */
    qcReader = (CHCQC_READER_T*)VERIQC_MALLOC(sizeof(CHCQC_READER_T));
    if (!qcReader)
    {
        m_lastErrorMsg = "<GNSSQC error>: Failed to create decoder/Input file does not exit.";
        return false;
    }
    memset(qcReader,0,sizeof(CHCQC_READER_T));
    QDir().mkpath(QString(config.OutDir));
    ret = InitQCReader(qcReader);
    if(ret == -1)
    {
        m_lastErrorMsg = "<GNSSQC error>: Failed to create decoder/Input file does not exit.";
        VERIQC_FREE(qcReader);
        return false;
    }
    InitRTKCore((double)config.EleDeg);
    BOOL state = SyncGNSSQCState(&config,qcReader);
    if(!state)
    {
        m_lastErrorMsg = "<GNSSQC error>: Failed to create decoder/Input file does not exit.";
        FreeQCReader(qcReader);
        return false;
    }
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
#else
    const QCConfigurator::RinexConvertSettings &rinexConvertSettings = AppSettingsManagement::getInstance()->getRinexConvertSettings();
    /** convert to QC API rinex convert config */
    CHCQC_RINEXCFG_T rinexConfig = m_configurator->generateAPIRinexConvertConfig(rinexConvertSettings);
#endif

    callback(name,0);
    RinexProcesser(&rinexConfig);
    callback(name,100);
    return true;
}

const QString &QCProcessor::getLastErrorMsg() const
{
    return m_lastErrorMsg;
}
