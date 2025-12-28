/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI QC Object Module
 * *-
 * @file QCResultManager.h
 * @author CHC
 * @date 2025-09-28
 * @brief Management QC result of QC Object 
 * 
**************************************************************************/
#ifndef QCRESULTMANAGER_H
#define QCRESULTMANAGER_H

#include <QList>
#include <QDateTime>
#include "VeriQC.h"

#define SYS_INDEX_GPS   0
#define SYS_INDEX_SBAS  1
#define SYS_INDEX_GLO   2
#define SYS_INDEX_GAL   3
#define SYS_INDEX_QZSS  4
#define SYS_INDEX_BDS   5

#define SYS_INDEX_MAX   (SYS_INDEX_BDS + 1)

#define FREQ_INDEX_GPS_L1   0
#define FREQ_INDEX_GPS_L2C  1
#define FREQ_INDEX_GPS_L5   2
#define FREQ_INDEX_GPS_L1C  3
#define FREQ_INDEX_GPS_L2P  4

#define FREQ_INDEX_SBAS_L1  0
#define FREQ_INDEX_SBAS_L5  1

#define FREQ_INDEX_GLO_G1   0
#define FREQ_INDEX_GLO_G2   1

#define FREQ_INDEX_GAL_E1   0
#define FREQ_INDEX_GAL_E5A  1
#define FREQ_INDEX_GAL_E5B  2
#define FREQ_INDEX_GAL_E5AB 3   // E5(A+B)
#define FREQ_INDEX_GAL_E6   4

#define FREQ_INDEX_QZSS_L1  0
#define FREQ_INDEX_QZSS_L2  1
#define FREQ_INDEX_QZSS_L5  2
#define FREQ_INDEX_QZSS_L1CP 3
#define FREQ_INDEX_QZSS_L6  4

#define FREQ_INDEX_BDS_B1   0
#define FREQ_INDEX_BDS_B2   1
#define FREQ_INDEX_BDS_B3   2
#define FREQ_INDEX_BDS_B1C  3
#define FREQ_INDEX_BDS_B2A  4
#define FREQ_INDEX_BDS_B2B  5


class QCResultManager
{
public:
    enum QCSysTypeEnum
    {
        QCSysComplex,
        QCSysGPS,
        QCSysBDS,
        QCSysGLO,
        QCSysGAL,
        QCSysQZSS,
    };

    enum FreqTypeEnum
    {
        GPSL1,
        GPSL2C,
        GPSL5,
        GPSL1C,
        GPSL2P,

        BDSB1,
        BDSB2,
        BDSB3,
        BDSB1C,
        BDSB2A,
        BDSB2B,

        GLOG1,
        GLOG2,

        GALE1,
        GALE5A,
        GALE5B,
        GALE5AB,
        GALE6,

        QZSSL1,
        QZSSL2,
        QZSSL5,
        QZSSL1CP,
        QZSSL6,
    };

    enum UiComplexTypeEnum
    {
        UiComplexMP1,   // Complex SNR is same order
        UiComplexMP2,
        UiComplexMP5,
        UiComplexMP6,
        UiComplexMP7,
        UiComplexMP8,
        UiComplexPlotExtend,    // extend complex freq for time plots
    };

#define UI_COMPLEX_MP_MAX_NUM   ((int)(QCResultManager::UiComplexMP8+1))
#define UI_MP_MAX_FREQ_NUM (UI_COMPLEX_MP_MAX_NUM > QC_NUM_FREQ ? UI_COMPLEX_MP_MAX_NUM : QC_NUM_FREQ)

    struct QCSysStatisticResultData
    {
        double useRate = 0;
        double cycleJumpRatio = 0;
        double MP[UI_MP_MAX_FREQ_NUM] = {0};     // >0: valid, <0: invalid
        double SNR[UI_MP_MAX_FREQ_NUM] = {0};    // >0: valid, <0: invalid
    };

    struct QCEpochSatObsData
    {
        unsigned char sat;			        /**< satellite/receiver number, sat index == sat - 1 */
        unsigned char sys, prn;
        unsigned char nfre;
        int frq;                            /**< GLONASS frq */

        double SNR[QC_NUM_FREQ];		    /**< signal strength (dbHz) */
        unsigned char LLI[QC_NUM_FREQ];		/**< loss of lock indicator */
        double L[QC_NUM_FREQ];				/**< observation data carrier-phase (cycle) */
        double P[QC_NUM_FREQ];				/**< observation data pseudorange (m) */
        float  D[QC_NUM_FREQ];				/**< observation data doppler frequency (m/s) */
        double azel[2];                     /**< azimuth and elevation(rad) */
        int slipFlag[QC_NUM_FREQ];          /**< satellite slip flag for every epoch */
        double curMP[QC_NUM_FREQ];          /**< current MP value */
    };

    struct QCEpochObsData                       /**< observation data */
    {
        QC_TIME_T time;					        /**< receiver sampling time (gpst) */
        double StaPos[3];                       /**< station postion(ECEF-XYZ) */
        QList<QCEpochSatObsData> data;          /**< observation data records */
    };

    struct QCEpochResult
    {
        QCEpochObsData epochObs;
        QC_SOL_INFO_T epochSolInfo;
    };
    struct StationAttributeInfo
    {
        double ecef[3] = {0};     // ECEF XYZ
        QDateTime startObsTime;
        QDateTime endObsTime;
        double interval = 0;
        unsigned int EpochCount = 0;
        QString receiverManufacturer;
        QString receiverType;
        QString receiverVersion;
        QString receiverSN;
        QString antennaManufacturer;
        QString antennaType;
        QString antennaSN;
        double AntDeltaH = 0;
    };

    struct GNSSSysFreqIndexInfoStruct
    {
        QCResultManager::QCSysTypeEnum sysType = QCResultManager::QCSysComplex;
        int sysIndex = 0;
        int freqIndex = 0;
    };

public:
    QCResultManager();
    void resetResult();
    void addEpochResult(const QCEpochResult &epochResult);
    void updateQCResult(const QC_RESULT_T &result);
    const QC_RESULT_T &getQCResult()const;
    const QList<QCEpochResult> &getEpochResultList()const;
    double getAllSysLossRate()const;
    QCSysStatisticResultData getSysStatisticResult(QCSysTypeEnum sys) const;
    QDateTime getStartTime()const;
    QDateTime getEndTime()const;
    static int getSystemIndex(QCSysTypeEnum systemType);
    static bool isSatValid(const QCEpochSatObsData &satData);
    static QString getSatName(QCSysTypeEnum sys, unsigned int prn);
    static QString getFreqName(QCSysTypeEnum sys, unsigned int freqIndex);
    static QString getFreqName(FreqTypeEnum type);
    static QString getUIComplexFreqsName(UiComplexTypeEnum type);
    static QVector<FreqTypeEnum> getUIComplexFreqTypes(UiComplexTypeEnum type);
    static GNSSSysFreqIndexInfoStruct getFreqIndexInfo(FreqTypeEnum type);
    static QDateTime qcTime2DateTime(QC_TIME_T t);
    QList<unsigned int> getValidSysSatPrns(QCSysTypeEnum sys);
    StationAttributeInfo getStationAttributeInfo()const;
    void updateStationAttributeInfo(const StationAttributeInfo &info);
protected:
    void calculateSysCycleJumpRatio();
private:
    QC_RESULT_T m_qcResult;
    QList<QCEpochResult> m_epochResultList;
    double m_sysCycleJumpRatio[QC_MAX_NUM_SYS] = {0};
    QMap<QCResultManager::QCSysTypeEnum,QSet<unsigned int>> m_validSysPrnMap;
    StationAttributeInfo m_stationAttributeInfo;
};

#endif // QCRESULTMANAGER_H
