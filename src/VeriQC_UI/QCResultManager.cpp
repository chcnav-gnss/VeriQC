#include "QCResultManager.h"
#include <QMap>
#include <math.h>


QCResultManager::QCResultManager()
{

}

void QCResultManager::resetResult()
{
    memset(&m_qcResult,0,sizeof(QC_RESULT_T));
    m_epochResultList.clear();
}

void QCResultManager::addEpochResult(const QCEpochResult &epochResult)
{
    m_epochResultList.append(epochResult);

    for(const QCResultManager::QCEpochSatObsData &satData : epochResult.epochObs.data)
    {
        if(!QCResultManager::isSatValid(satData))
        {
            continue;
        }
        switch (satData.sys)
        {
        case QC_SYS_GPS:
            m_validSysPrnMap[QCResultManager::QCSysGPS].insert(satData.prn);
            break;
        case QC_SYS_CMP:
            m_validSysPrnMap[QCResultManager::QCSysBDS].insert(satData.prn);
            break;
        case QC_SYS_GLO:
            m_validSysPrnMap[QCResultManager::QCSysGLO].insert(satData.prn);
            break;
        case QC_SYS_GAL:
            m_validSysPrnMap[QCResultManager::QCSysGAL].insert(satData.prn);
            break;
        case QC_SYS_QZS:
            m_validSysPrnMap[QCResultManager::QCSysQZSS].insert(satData.prn);
            break;
        default:
            break;
        }
    }
}

void QCResultManager::updateQCResult(const QC_RESULT_T &result)
{
    m_qcResult = result;
    this->calculateSysCycleJumpRatio();
}

const QC_RESULT_T &QCResultManager::getQCResult() const
{
    return m_qcResult;
}

const QList<QCResultManager::QCEpochResult> &QCResultManager::getEpochResultList() const
{
    return m_epochResultList;
}

double QCResultManager::getAllSysLossRate() const
{
    int32_t calculateEpochNum = std::round((m_qcResult.EndTime[0].Time + m_qcResult.EndTime[0].Sec - m_qcResult.StartTime[0].Time - m_qcResult.StartTime[0].Sec) / m_qcResult.Interval[0]) + 1;
    return (1 - m_qcResult.MaxEpochNum[0] * 1.0 / calculateEpochNum) * 100;
}

QCResultManager::QCSysStatisticResultData QCResultManager::getSysStatisticResult(QCSysTypeEnum sys) const
{
    QCSysStatisticResultData data;
    if(QCSysTypeEnum::QCSysComplex == sys)
    {
        data.useRate = m_qcResult.UseRateTotal[0] * 100;
        data.cycleJumpRatio = m_qcResult.OSR[0];
        for (int i = (int)QCResultManager::UiComplexMP1; i <= (int)QCResultManager::UiComplexMP8; ++i)
        {
            double totalMPValue = 0;
            double totalSNRValue = 0;
            int countMP = 0;
            int countSNR = 0;
            QVector<QCResultManager::FreqTypeEnum> freqs = getUIComplexFreqTypes((QCResultManager::UiComplexTypeEnum)i);
            for(QCResultManager::FreqTypeEnum freq : freqs)
            {
                QCResultManager::GNSSSysFreqIndexInfoStruct freqInfo = getFreqIndexInfo(freq);
                float mp = m_qcResult.MPTotel[0][freqInfo.sysIndex][freqInfo.freqIndex];
                float snr = m_qcResult.SNR[0][freqInfo.sysIndex][freqInfo.freqIndex];
                if(mp > 0)
                {
                    totalMPValue += mp;
                    countMP++;
                }
                if(snr > 0)
                {
                    totalSNRValue += snr;
                    countSNR++;
                }

            }

            if(countMP > 0)
            {
                data.MP[i] = totalMPValue/countMP;
            }
            if(countSNR >0)
            {
                data.SNR[i] = totalSNRValue/countSNR;
            }
        }
    }
    else
    {
        int sysIndex = this->getSystemIndex(sys);
        if(sysIndex < 0)
        {
            return data;
        }

        data.useRate = m_qcResult.UseRateSys[0][sysIndex] * 100;
        data.cycleJumpRatio = m_sysCycleJumpRatio[sysIndex];
        for (int i = (int)QCResultManager::UiComplexMP1; i <= (int)QCResultManager::UiComplexMP8; ++i)
        {
            QVector<QCResultManager::FreqTypeEnum> freqs = getUIComplexFreqTypes((QCResultManager::UiComplexTypeEnum)i);
            for (QCResultManager::FreqTypeEnum freq : freqs)
            {
                QCResultManager::GNSSSysFreqIndexInfoStruct freqInfo = getFreqIndexInfo(freq);
                if (freqInfo.sysIndex == sysIndex)
                {
                    data.MP[i] = m_qcResult.MPTotel[0][sysIndex][freqInfo.freqIndex];
                    data.SNR[i] = m_qcResult.SNR[0][sysIndex][freqInfo.freqIndex];
                    break;
                }
            }
        }
    }
    return data;
}

QDateTime QCResultManager::getStartTime() const
{
    return qcTime2DateTime(m_qcResult.StartTime[0]);
}

QDateTime QCResultManager::getEndTime() const
{
    return qcTime2DateTime(m_qcResult.EndTime[0]);
}

int QCResultManager::getSystemIndex(QCSysTypeEnum systemType)
{
    static QHash<QCSysTypeEnum,int> s_sysIndexHash = {
        {QCSysGPS,SYS_INDEX_GPS},
        {QCSysBDS,SYS_INDEX_BDS},
        {QCSysGLO,SYS_INDEX_GLO},
        {QCSysGAL,SYS_INDEX_GAL},
        {QCSysQZSS,SYS_INDEX_QZSS}
    };
    return s_sysIndexHash.value(systemType,-1);
}

bool QCResultManager::isSatValid(const QCEpochSatObsData &satData)
{
    bool valid = false;
    for (int i = 0; i < QC_NUM_FREQ; i++)
    {
        if ((satData.L[i] != 0) || (satData.P[i] != 0))
        {
            valid = true;
            break;
        }
    }
    return valid;
}

QString QCResultManager::getSatName(QCSysTypeEnum sys, unsigned int prn)
{
    static QHash<QCSysTypeEnum,QString> s_sysNameHash = {
        {QCSysGPS,"G"},
        {QCSysGLO,"R"},
        {QCSysGAL,"E"},
        {QCSysQZSS,"J"},
        {QCSysBDS,"C"}
    };

    return QString("%1%2").arg(s_sysNameHash.value(sys,"NA")).arg(prn, 2, 10, QChar('0'));
}

QString QCResultManager::getFreqName(QCSysTypeEnum sys, unsigned int freqIndex)
{
    static char s_sigType[QC_MAX_NUM_SYS][QC_NUM_FREQ][16] = {
        {"L1","L2C","L5","L1C","L2P",""},/*GPS*/
        {"L1","L5",  "",     "",     "",""},/*SBAS*/
        {"G1","G2","", "",   "",""},/*GLONASS*/
        {"E1","E5a","E5b","E5(a+b)","E6",""},/*Galileo*/
        {"L1","L2","L5","L1C(P)","L6",""},/*QZSS*/
        {"B1","B2","B3","B1C","B2a","B2b"}/*BDS*/
    };

    Q_ASSERT(freqIndex < QC_NUM_FREQ);
    if(freqIndex < QC_NUM_FREQ)
    {
        if(QCSysComplex != sys)
        {
            return s_sigType[getSystemIndex(sys)][freqIndex];
        }
    }
    return QString();
}

QString QCResultManager::getFreqName(FreqTypeEnum type)
{
    GNSSSysFreqIndexInfoStruct freqInfo = getFreqIndexInfo(type);
    return getFreqName(freqInfo.sysType,freqInfo.freqIndex);
}

QString QCResultManager::getUIComplexFreqsName(UiComplexTypeEnum type)
{
    QStringList names;
    for(QCResultManager::FreqTypeEnum freqType : getUIComplexFreqTypes(type))
    {
        GNSSSysFreqIndexInfoStruct sysFreqIndex = getFreqIndexInfo(freqType);
        QString freqName = getFreqName(sysFreqIndex.sysType,sysFreqIndex.freqIndex);
        names.append(freqName);
    }

    return names.join("/");
}

QVector<QCResultManager::FreqTypeEnum> QCResultManager::getUIComplexFreqTypes(UiComplexTypeEnum type)
{
    static const QHash<QCResultManager::UiComplexTypeEnum,QVector<QCResultManager::FreqTypeEnum>> s_uiComplexFreqHash = {
        {QCResultManager::UiComplexMP1,{QCResultManager::GPSL1,QCResultManager::GLOG1,QCResultManager::GALE1,QCResultManager::BDSB1C,QCResultManager::QZSSL1}},
        {QCResultManager::UiComplexMP2,{QCResultManager::GPSL2P,QCResultManager::GLOG2,QCResultManager::BDSB1,QCResultManager::QZSSL2}},
        {QCResultManager::UiComplexMP5,{QCResultManager::GPSL5,QCResultManager::GALE5A,QCResultManager::BDSB2A,QCResultManager::QZSSL5}},
        {QCResultManager::UiComplexMP6,{QCResultManager::GALE6,QCResultManager::BDSB3,QCResultManager::QZSSL6}},
        {QCResultManager::UiComplexMP7,{QCResultManager::GALE5B,QCResultManager::BDSB2}},
        {QCResultManager::UiComplexMP8,{QCResultManager::GALE5AB}},
        {QCResultManager::UiComplexPlotExtend,{QCResultManager::GPSL1C,QCResultManager::QZSSL1CP,QCResultManager::BDSB2B}},
    };

    return s_uiComplexFreqHash[type];
}

QCResultManager::GNSSSysFreqIndexInfoStruct QCResultManager::getFreqIndexInfo(FreqTypeEnum type)
{
    static const QHash<QCResultManager::FreqTypeEnum,GNSSSysFreqIndexInfoStruct> s_gnssSysFreqIndexMap = {
        {QCResultManager::FreqTypeEnum::GPSL1,{QCResultManager::QCSysGPS,SYS_INDEX_GPS,FREQ_INDEX_GPS_L1}},
        {QCResultManager::FreqTypeEnum::GPSL2C,{QCResultManager::QCSysGPS,SYS_INDEX_GPS,FREQ_INDEX_GPS_L2C}},
        {QCResultManager::FreqTypeEnum::GPSL5,{QCResultManager::QCSysGPS,SYS_INDEX_GPS,FREQ_INDEX_GPS_L5}},
        {QCResultManager::FreqTypeEnum::GPSL1C,{QCResultManager::QCSysGPS,SYS_INDEX_GPS,FREQ_INDEX_GPS_L1C}},
        {QCResultManager::FreqTypeEnum::GPSL2P,{QCResultManager::QCSysGPS,SYS_INDEX_GPS,FREQ_INDEX_GPS_L2P}},

        {QCResultManager::FreqTypeEnum::BDSB1,{QCResultManager::QCSysBDS,SYS_INDEX_BDS,FREQ_INDEX_BDS_B1}},
        {QCResultManager::FreqTypeEnum::BDSB2,{QCResultManager::QCSysBDS,SYS_INDEX_BDS,FREQ_INDEX_BDS_B2}},
        {QCResultManager::FreqTypeEnum::BDSB3,{QCResultManager::QCSysBDS,SYS_INDEX_BDS,FREQ_INDEX_BDS_B3}},
        {QCResultManager::FreqTypeEnum::BDSB1C,{QCResultManager::QCSysBDS,SYS_INDEX_BDS,FREQ_INDEX_BDS_B1C}},
        {QCResultManager::FreqTypeEnum::BDSB2A,{QCResultManager::QCSysBDS,SYS_INDEX_BDS,FREQ_INDEX_BDS_B2A}},
        {QCResultManager::FreqTypeEnum::BDSB2B,{QCResultManager::QCSysBDS,SYS_INDEX_BDS,FREQ_INDEX_BDS_B2B}},

        {QCResultManager::FreqTypeEnum::GLOG1,{QCResultManager::QCSysGLO,SYS_INDEX_GLO,FREQ_INDEX_GLO_G1}},
        {QCResultManager::FreqTypeEnum::GLOG2,{QCResultManager::QCSysGLO,SYS_INDEX_GLO,FREQ_INDEX_GLO_G2}},

        {QCResultManager::FreqTypeEnum::GALE1,{QCResultManager::QCSysGAL,SYS_INDEX_GAL,FREQ_INDEX_GAL_E1}},
        {QCResultManager::FreqTypeEnum::GALE5A,{QCResultManager::QCSysGAL,SYS_INDEX_GAL,FREQ_INDEX_GAL_E5A}},
        {QCResultManager::FreqTypeEnum::GALE5B,{QCResultManager::QCSysGAL,SYS_INDEX_GAL,FREQ_INDEX_GAL_E5B}},
        {QCResultManager::FreqTypeEnum::GALE5AB,{QCResultManager::QCSysGAL,SYS_INDEX_GAL,FREQ_INDEX_GAL_E5AB}},
        {QCResultManager::FreqTypeEnum::GALE6,{QCResultManager::QCSysGAL,SYS_INDEX_GAL,FREQ_INDEX_GAL_E6}},

        {QCResultManager::FreqTypeEnum::QZSSL1,{QCResultManager::QCSysQZSS,SYS_INDEX_QZSS,FREQ_INDEX_QZSS_L1}},
        {QCResultManager::FreqTypeEnum::QZSSL2,{QCResultManager::QCSysQZSS,SYS_INDEX_QZSS,FREQ_INDEX_QZSS_L2}},
        {QCResultManager::FreqTypeEnum::QZSSL5,{QCResultManager::QCSysQZSS,SYS_INDEX_QZSS,FREQ_INDEX_QZSS_L5}},
        {QCResultManager::FreqTypeEnum::QZSSL1CP,{QCResultManager::QCSysQZSS,SYS_INDEX_QZSS,FREQ_INDEX_QZSS_L1CP}},
        {QCResultManager::FreqTypeEnum::QZSSL6,{QCResultManager::QCSysQZSS,SYS_INDEX_QZSS,FREQ_INDEX_QZSS_L6}},

    };

    return s_gnssSysFreqIndexMap[type];
}

QDateTime QCResultManager::qcTime2DateTime(QC_TIME_T t)
{
    double ep[6];
    char tstr[64] = {0};

    QCTime2Epoch(t, ep);
    sprintf(tstr, "%04.0f/%02.0f/%02.0f %02.0f:%02.0f:%06.3f", ep[0], ep[1], ep[2],
        ep[3], ep[4], ep[5]);
    return QDateTime::fromString(tstr,"yyyy/MM/dd HH:mm:ss.zzz");
}

QList<unsigned int> QCResultManager::getValidSysSatPrns(QCSysTypeEnum sys)
{
    QList<unsigned int> list = m_validSysPrnMap.value(sys).toList();
    std::sort(list.begin(),list.end());
    return list;
}

QCResultManager::StationAttributeInfo QCResultManager::getStationAttributeInfo() const
{
    return m_stationAttributeInfo;
}

void QCResultManager::updateStationAttributeInfo(const StationAttributeInfo &info)
{
    m_stationAttributeInfo = info;
}

void QCResultManager::calculateSysCycleJumpRatio()
{
    for (int i = 0; i < QC_MAX_NUM_SYS; ++i)
    {
        if (m_qcResult.SysDetectNum[0][i] > 0 && m_qcResult.SysSlipNum[0][i] > 0)
        {
            m_sysCycleJumpRatio[i] = 1.0 * m_qcResult.SysDetectNum[0][i] / m_qcResult.SysSlipNum[0][i];
        }
        else
        {
            m_sysCycleJumpRatio[i] = 99999.0;
        }
    }

}
