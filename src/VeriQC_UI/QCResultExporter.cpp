#include "QCResultExporter.h"
#include <QObject>
#include "QCObjectManager.h"
#include "xlsxdocument.h"

enum ExcelColumnEnum
{
    ExcelColumnFileName = 1,
    ExcelColumnStartTime,
    ExcelColumnEndTime,
    ExcelColumnEleDeg,

    ExcelColumnAllLossRate,
    ExcelColumnAllUseRate,
    ExcelColumnAllCJR,
    ExcelColumnAllMP1,
    ExcelColumnAllMP2,
    ExcelColumnAllMP5,
    ExcelColumnAllMP6,
    ExcelColumnAllMP7,
    ExcelColumnAllMP8,
    ExcelColumnAllSNR1,
    ExcelColumnAllSNR2,
    ExcelColumnAllSNR5,
    ExcelColumnAllSNR6,
    ExcelColumnAllSNR7,
    ExcelColumnAllSNR8,

    ExcelColumnBDSUseRate,
    ExcelColumnBDSCJR,
    ExcelColumnBDSMPB1,
    ExcelColumnBDSMPB2,
    ExcelColumnBDSMPB3,
    ExcelColumnBDSMPB1C,
    ExcelColumnBDSMPB2A,
    ExcelColumnBDSMPB2B,
    ExcelColumnBDSSNRB1,
    ExcelColumnBDSSNRB2,
    ExcelColumnBDSSNRB3,
    ExcelColumnBDSSNRB1C,
    ExcelColumnBDSSNRB2A,
    ExcelColumnBDSSNRB2B,

    ExcelColumnGPSUseRate,
    ExcelColumnGPSCJR,
    ExcelColumnGPSMPL1,
    ExcelColumnGPSMPL2C,
    ExcelColumnGPSMPL5,
    ExcelColumnGPSMPL1C,
    ExcelColumnGPSMPL2P,
    ExcelColumnGPSSNRL1,
    ExcelColumnGPSSNRL2C,
    ExcelColumnGPSSNRL5,
    ExcelColumnGPSSNRL1C,
    ExcelColumnGPSSNRL2P,

    ExcelColumnGALUseRate,
    ExcelColumnGALCJR,
    ExcelColumnGALMPE1,
    ExcelColumnGALMPE5A,
    ExcelColumnGALMPE5B,
    ExcelColumnGALMPE5AB,
    ExcelColumnGALMPE6,
    ExcelColumnGALSNRE1,
    ExcelColumnGALSNRE5A,
    ExcelColumnGALSNRE5B,
    ExcelColumnGALSNRE5AB,
    ExcelColumnGALSNRE6,

    ExcelColumnGLOUseRate,
    ExcelColumnGLOCJR,
    ExcelColumnGLOMPG1,
    ExcelColumnGLOMPG2,
    ExcelColumnGLOSNRG1,
    ExcelColumnGLOSNRG2,

    ExcelColumnQZSSUseRate,
    ExcelColumnQZSSCJR,
    ExcelColumnQZSSMPL1,
    ExcelColumnQZSSMPL2,
    ExcelColumnQZSSMPL5,
    ExcelColumnQZSSMPL1CP,
    ExcelColumnQZSSMPL6,
    ExcelColumnQZSSSNRL1,
    ExcelColumnQZSSSNRL2,
    ExcelColumnQZSSSNRL5,
    ExcelColumnQZSSSNRL1CP,
    ExcelColumnQZSSSNRL6,
};


QCResultExporter::QCResultExporter()
{

}

bool QCResultExporter::exportToExcel(const QString &filePath, const QVector<int> &qcObjects)
{
    using namespace QXlsx;

    // create excel
    Document xlsx;
    Worksheet *sheet = xlsx.currentWorksheet();

    // init header
    sheet->write(1, ExcelColumnFileName, QObject::tr("File Name"));
    sheet->mergeCells(CellRange(1,ExcelColumnFileName,3,ExcelColumnFileName));
    sheet->write(1, ExcelColumnStartTime, QObject::tr("Start Time"));
    sheet->mergeCells(CellRange(1,ExcelColumnStartTime,3,ExcelColumnStartTime));
    sheet->write(1, ExcelColumnEndTime, QObject::tr("End Time"));
    sheet->mergeCells(CellRange(1,ExcelColumnEndTime,3,ExcelColumnEndTime));
    sheet->write(1, ExcelColumnEleDeg, QObject::tr("Elevation Angle"));
    sheet->mergeCells(CellRange(1,ExcelColumnEleDeg,3,ExcelColumnEleDeg));

    // ALL header
    sheet->write(1, ExcelColumnAllLossRate, QObject::tr("ALL"));
    sheet->mergeCells(CellRange(1,ExcelColumnAllLossRate,1,ExcelColumnAllSNR8));   // additional Loss Rate field
    sheet->write(2, ExcelColumnAllLossRate, QObject::tr("Loss Rate"));
    sheet->mergeCells(CellRange(2,ExcelColumnAllLossRate,3,ExcelColumnAllLossRate));
    sheet->write(2, ExcelColumnAllUseRate, QObject::tr("Use Rate"));
    sheet->mergeCells(CellRange(2,ExcelColumnAllUseRate,3,ExcelColumnAllUseRate));
    sheet->write(2, ExcelColumnAllCJR, QObject::tr("Cycle Jump Ratio"));
    sheet->mergeCells(CellRange(2,ExcelColumnAllCJR,3,ExcelColumnAllCJR));
    sheet->write(2, ExcelColumnAllMP1, QObject::tr("MP(m)"));
    sheet->mergeCells(CellRange(2,ExcelColumnAllMP1,2,ExcelColumnAllMP8));

    sheet->write(3, ExcelColumnAllMP1, QString("MP1(%1)").arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP1)));
    sheet->write(3, ExcelColumnAllMP2, QString("MP2(%1)").arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP2)));
    sheet->write(3, ExcelColumnAllMP5, QString("MP5(%1)").arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP5)));
    sheet->write(3, ExcelColumnAllMP6, QString("MP6(%1)").arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP6)));
    sheet->write(3, ExcelColumnAllMP7, QString("MP7(%1)").arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP7)));
    sheet->write(3, ExcelColumnAllMP8, QString("MP8(%1)").arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP8)));

    sheet->write(2, ExcelColumnAllSNR1, QObject::tr("SNR"));
    sheet->mergeCells(CellRange(2,ExcelColumnAllSNR1,2,ExcelColumnAllSNR8));

    sheet->write(3, ExcelColumnAllSNR1, QString("SNR1(%1)").arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP1)));
    sheet->write(3, ExcelColumnAllSNR2, QString("SNR2(%1)").arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP2)));
    sheet->write(3, ExcelColumnAllSNR5, QString("SNR5(%1)").arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP5)));
    sheet->write(3, ExcelColumnAllSNR6, QString("SNR6(%1)").arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP6)));
    sheet->write(3, ExcelColumnAllSNR7, QString("SNR7(%1)").arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP7)));
    sheet->write(3, ExcelColumnAllSNR8, QString("SNR8(%1)").arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP8)));
    //BDS header
    sheet->write(1, ExcelColumnBDSUseRate, QObject::tr("BDS"));
    sheet->mergeCells(CellRange(1,ExcelColumnBDSUseRate,1,ExcelColumnBDSSNRB2B));
    sheet->write(2, ExcelColumnBDSUseRate, QObject::tr("Use Rate"));
    sheet->mergeCells(CellRange(2,ExcelColumnBDSUseRate,3,ExcelColumnBDSUseRate));
    sheet->write(2, ExcelColumnBDSCJR, QObject::tr("Cycle Jump Ratio"));
    sheet->mergeCells(CellRange(2,ExcelColumnBDSCJR,3,ExcelColumnBDSCJR));
    sheet->write(2, ExcelColumnBDSMPB1, QObject::tr("MP(m)"));
    sheet->mergeCells(CellRange(2,ExcelColumnBDSMPB1,2,ExcelColumnBDSMPB2B));

    sheet->write(3, ExcelColumnBDSMPB1, QCResultManager::getFreqName(QCResultManager::BDSB1));
    sheet->write(3, ExcelColumnBDSMPB2, QCResultManager::getFreqName(QCResultManager::BDSB2));
    sheet->write(3, ExcelColumnBDSMPB3, QCResultManager::getFreqName(QCResultManager::BDSB3));
    sheet->write(3, ExcelColumnBDSMPB1C, QCResultManager::getFreqName(QCResultManager::BDSB1C));
    sheet->write(3, ExcelColumnBDSMPB2A, QCResultManager::getFreqName(QCResultManager::BDSB2A));
    sheet->write(3, ExcelColumnBDSMPB2B, QCResultManager::getFreqName(QCResultManager::BDSB2B));

    sheet->write(2, ExcelColumnBDSSNRB1, QObject::tr("SNR"));
    sheet->mergeCells(CellRange(2,ExcelColumnBDSSNRB1,2,ExcelColumnBDSSNRB2B));

    sheet->write(3, ExcelColumnBDSSNRB1, QCResultManager::getFreqName(QCResultManager::BDSB1));
    sheet->write(3, ExcelColumnBDSSNRB2, QCResultManager::getFreqName(QCResultManager::BDSB2));
    sheet->write(3, ExcelColumnBDSSNRB3, QCResultManager::getFreqName(QCResultManager::BDSB3));
    sheet->write(3, ExcelColumnBDSSNRB1C, QCResultManager::getFreqName(QCResultManager::BDSB1C));
    sheet->write(3, ExcelColumnBDSSNRB2A, QCResultManager::getFreqName(QCResultManager::BDSB2A));
    sheet->write(3, ExcelColumnBDSSNRB2B, QCResultManager::getFreqName(QCResultManager::BDSB2B));

    //GPS header
    sheet->write(1, ExcelColumnGPSUseRate, QObject::tr("GPS"));
    sheet->mergeCells(CellRange(1,ExcelColumnGPSUseRate,1,ExcelColumnGPSSNRL2P));
    sheet->write(2, ExcelColumnGPSUseRate, QObject::tr("Use Rate"));
    sheet->mergeCells(CellRange(2,ExcelColumnGPSUseRate,3,ExcelColumnGPSUseRate));
    sheet->write(2, ExcelColumnGPSCJR, QObject::tr("Cycle Jump Ratio"));
    sheet->mergeCells(CellRange(2,ExcelColumnGPSCJR,3,ExcelColumnGPSCJR));
    sheet->write(2, ExcelColumnGPSMPL1, QObject::tr("MP(m)"));
    sheet->mergeCells(CellRange(2,ExcelColumnGPSMPL1,2,ExcelColumnGPSMPL2P));

    sheet->write(3, ExcelColumnGPSMPL1, QCResultManager::getFreqName(QCResultManager::GPSL1));
    sheet->write(3, ExcelColumnGPSMPL2C, QCResultManager::getFreqName(QCResultManager::GPSL2C));
    sheet->write(3, ExcelColumnGPSMPL5, QCResultManager::getFreqName(QCResultManager::GPSL5));
    sheet->write(3, ExcelColumnGPSMPL1C, QCResultManager::getFreqName(QCResultManager::GPSL1C));
    sheet->write(3, ExcelColumnGPSMPL2P, QCResultManager::getFreqName(QCResultManager::GPSL2P));

    sheet->write(2, ExcelColumnGPSSNRL1, QObject::tr("SNR"));
    sheet->mergeCells(CellRange(2,ExcelColumnGPSSNRL1,2,ExcelColumnGPSSNRL2P));

    sheet->write(3, ExcelColumnGPSSNRL1, QCResultManager::getFreqName(QCResultManager::GPSL1));
    sheet->write(3, ExcelColumnGPSSNRL2C, QCResultManager::getFreqName(QCResultManager::GPSL2C));
    sheet->write(3, ExcelColumnGPSSNRL5, QCResultManager::getFreqName(QCResultManager::GPSL5));
    sheet->write(3, ExcelColumnGPSSNRL1C, QCResultManager::getFreqName(QCResultManager::GPSL1C));
    sheet->write(3, ExcelColumnGPSSNRL2P, QCResultManager::getFreqName(QCResultManager::GPSL2P));

    //GAL header
    sheet->write(1, ExcelColumnGALUseRate, QObject::tr("GAL"));
    sheet->mergeCells(CellRange(1,ExcelColumnGALUseRate,1,ExcelColumnGALSNRE6));
    sheet->write(2, ExcelColumnGALUseRate, QObject::tr("Use Rate"));
    sheet->mergeCells(CellRange(2,ExcelColumnGALUseRate,3,ExcelColumnGALUseRate));
    sheet->write(2, ExcelColumnGALCJR, QObject::tr("Cycle Jump Ratio"));
    sheet->mergeCells(CellRange(2,ExcelColumnGALCJR,3,ExcelColumnGALCJR));
    sheet->write(2, ExcelColumnGALMPE1, QObject::tr("MP(m)"));
    sheet->mergeCells(CellRange(2,ExcelColumnGALMPE1,2,ExcelColumnGALMPE6));

    sheet->write(3, ExcelColumnGALMPE1, QCResultManager::getFreqName(QCResultManager::GALE1));
    sheet->write(3, ExcelColumnGALMPE5A, QCResultManager::getFreqName(QCResultManager::GALE5A));
    sheet->write(3, ExcelColumnGALMPE5B, QCResultManager::getFreqName(QCResultManager::GALE5B));
    sheet->write(3, ExcelColumnGALMPE5AB, QCResultManager::getFreqName(QCResultManager::GALE5AB));
    sheet->write(3, ExcelColumnGALMPE6, QCResultManager::getFreqName(QCResultManager::GALE6));

    sheet->write(2, ExcelColumnGALSNRE1, QObject::tr("SNR"));
    sheet->mergeCells(CellRange(2,ExcelColumnGALSNRE1,2,ExcelColumnGALSNRE6));

    sheet->write(3, ExcelColumnGALSNRE1, QCResultManager::getFreqName(QCResultManager::GALE1));
    sheet->write(3, ExcelColumnGALSNRE5A, QCResultManager::getFreqName(QCResultManager::GALE5A));
    sheet->write(3, ExcelColumnGALSNRE5B, QCResultManager::getFreqName(QCResultManager::GALE5B));
    sheet->write(3, ExcelColumnGALSNRE5AB, QCResultManager::getFreqName(QCResultManager::GALE5AB));
    sheet->write(3, ExcelColumnGALSNRE6, QCResultManager::getFreqName(QCResultManager::GALE6));

    //GLO header
    sheet->write(1, ExcelColumnGLOUseRate, QObject::tr("GLO"));
    sheet->mergeCells(CellRange(1,ExcelColumnGLOUseRate,1,ExcelColumnGLOSNRG2));
    sheet->write(2, ExcelColumnGLOUseRate, QObject::tr("Use Rate"));
    sheet->mergeCells(CellRange(2,ExcelColumnGLOUseRate,3,ExcelColumnGLOUseRate));
    sheet->write(2, ExcelColumnGLOCJR, QObject::tr("Cycle Jump Ratio"));
    sheet->mergeCells(CellRange(2,ExcelColumnGLOCJR,3,ExcelColumnGLOCJR));
    sheet->write(2, ExcelColumnGLOMPG1, QObject::tr("MP(m)"));
    sheet->mergeCells(CellRange(2,ExcelColumnGLOMPG1,2,ExcelColumnGLOMPG2));

    sheet->write(3, ExcelColumnGLOMPG1, QCResultManager::getFreqName(QCResultManager::GLOG1));
    sheet->write(3, ExcelColumnGLOMPG2, QCResultManager::getFreqName(QCResultManager::GLOG2));

    sheet->write(2, ExcelColumnGLOSNRG1, QObject::tr("SNR"));
    sheet->mergeCells(CellRange(2,ExcelColumnGLOSNRG1,2,ExcelColumnGLOSNRG2));

    sheet->write(3, ExcelColumnGLOSNRG1, QCResultManager::getFreqName(QCResultManager::GLOG1));
    sheet->write(3, ExcelColumnGLOSNRG2, QCResultManager::getFreqName(QCResultManager::GLOG2));

    //QZSS header
    sheet->write(1, ExcelColumnQZSSUseRate, QObject::tr("QZSS"));
    sheet->mergeCells(CellRange(1,ExcelColumnQZSSUseRate,1,ExcelColumnQZSSSNRL6));
    sheet->write(2, ExcelColumnQZSSUseRate, QObject::tr("Use Rate"));
    sheet->mergeCells(CellRange(2,ExcelColumnQZSSUseRate,3,ExcelColumnQZSSUseRate));
    sheet->write(2, ExcelColumnQZSSCJR, QObject::tr("Cycle Jump Ratio"));
    sheet->mergeCells(CellRange(2,ExcelColumnQZSSCJR,3,ExcelColumnQZSSCJR));
    sheet->write(2, ExcelColumnQZSSMPL1, QObject::tr("MP(m)"));
    sheet->mergeCells(CellRange(2,ExcelColumnQZSSMPL1,2,ExcelColumnQZSSMPL6));

    sheet->write(3, ExcelColumnQZSSMPL1, QCResultManager::getFreqName(QCResultManager::QZSSL1));
    sheet->write(3, ExcelColumnQZSSMPL2, QCResultManager::getFreqName(QCResultManager::QZSSL2));
    sheet->write(3, ExcelColumnQZSSMPL5, QCResultManager::getFreqName(QCResultManager::QZSSL5));
    sheet->write(3, ExcelColumnQZSSMPL1CP, QCResultManager::getFreqName(QCResultManager::QZSSL1CP));
    sheet->write(3, ExcelColumnQZSSMPL6, QCResultManager::getFreqName(QCResultManager::QZSSL6));

    sheet->write(2, ExcelColumnQZSSSNRL1, QObject::tr("SNR"));
    sheet->mergeCells(CellRange(2,ExcelColumnQZSSSNRL1,2,ExcelColumnQZSSSNRL6));

    sheet->write(3, ExcelColumnQZSSSNRL1, QCResultManager::getFreqName(QCResultManager::QZSSL1));
    sheet->write(3, ExcelColumnQZSSSNRL2, QCResultManager::getFreqName(QCResultManager::QZSSL2));
    sheet->write(3, ExcelColumnQZSSSNRL5, QCResultManager::getFreqName(QCResultManager::QZSSL5));
    sheet->write(3, ExcelColumnQZSSSNRL1CP, QCResultManager::getFreqName(QCResultManager::QZSSL1CP));
    sheet->write(3, ExcelColumnQZSSSNRL6, QCResultManager::getFreqName(QCResultManager::QZSSL6));


    // fill data
    int row = 4;
    for(int id : qcObjects)
    {
        QSharedPointer<QCObject> qcObject = QCObjectManager::getInstance()->getQCObjectByID(id);
        if(!qcObject)
        {
            continue;
        }
        bool qcExecuted = qcObject->getQCStatus() != QCObject::QCResultStatusEnum::QCStatusNotExecuted;

        sheet->write(row,ExcelColumnFileName,qcObject->getName());
        sheet->write(row,ExcelColumnStartTime,qcObject->getQCResultManager()->getStartTime().toString("yyyy/MM/dd HH:mm:ss"));
        sheet->write(row,ExcelColumnEndTime,qcObject->getQCResultManager()->getEndTime().toString("yyyy/MM/dd HH:mm:ss"));
        sheet->write(row,ExcelColumnEleDeg,qcObject->getQCConfigurator()->getQCConfig().eleDeg);

        //ALL
        auto allInfo = qcObject->getQCResultManager()->getSysStatisticResult(QCResultManager::QCSysTypeEnum::QCSysComplex);
        sheet->write(row,ExcelColumnAllLossRate,QString("%1%").arg(qcObject->getQCResultManager()->getAllSysLossRate(),0,'f',1));
        sheet->write(row,ExcelColumnAllUseRate,getDataStr(qcExecuted, allInfo.useRate,1,"%"));
        sheet->write(row,ExcelColumnAllCJR,getDataStr(qcExecuted, allInfo.cycleJumpRatio,0));
        sheet->write(row,ExcelColumnAllMP1,getDataStr(qcExecuted, allInfo.MP[(int)QCResultManager::UiComplexMP1],3));
        sheet->write(row,ExcelColumnAllMP2,getDataStr(qcExecuted, allInfo.MP[(int)QCResultManager::UiComplexMP2],3));
        sheet->write(row,ExcelColumnAllMP5,getDataStr(qcExecuted, allInfo.MP[(int)QCResultManager::UiComplexMP5],3));
        sheet->write(row,ExcelColumnAllMP6,getDataStr(qcExecuted, allInfo.MP[(int)QCResultManager::UiComplexMP6],3));
        sheet->write(row,ExcelColumnAllMP7,getDataStr(qcExecuted, allInfo.MP[(int)QCResultManager::UiComplexMP7],3));
        sheet->write(row,ExcelColumnAllMP8,getDataStr(qcExecuted, allInfo.MP[(int)QCResultManager::UiComplexMP8],3));
        sheet->write(row,ExcelColumnAllSNR1,getDataStr(qcExecuted, allInfo.SNR[(int)QCResultManager::UiComplexMP1],1));
        sheet->write(row,ExcelColumnAllSNR2,getDataStr(qcExecuted, allInfo.SNR[(int)QCResultManager::UiComplexMP2],1));
        sheet->write(row,ExcelColumnAllSNR5,getDataStr(qcExecuted, allInfo.SNR[(int)QCResultManager::UiComplexMP5],1));
        sheet->write(row,ExcelColumnAllSNR6,getDataStr(qcExecuted, allInfo.SNR[(int)QCResultManager::UiComplexMP6],1));
        sheet->write(row,ExcelColumnAllSNR7,getDataStr(qcExecuted, allInfo.SNR[(int)QCResultManager::UiComplexMP7],1));
        sheet->write(row,ExcelColumnAllSNR8,getDataStr(qcExecuted, allInfo.SNR[(int)QCResultManager::UiComplexMP8],1));

        //BDS
        auto bdsInfo = qcObject->getQCResultManager()->getSysStatisticResult(QCResultManager::QCSysTypeEnum::QCSysBDS);
        sheet->write(row,ExcelColumnBDSUseRate,getDataStr(qcExecuted, bdsInfo.useRate,1,"%"));
        sheet->write(row,ExcelColumnBDSCJR,getDataStr(qcExecuted, bdsInfo.cycleJumpRatio,0));
        sheet->write(row,ExcelColumnBDSMPB1,getDataStr(qcExecuted, bdsInfo.MP[FREQ_INDEX_BDS_B1],3));
        sheet->write(row,ExcelColumnBDSMPB2,getDataStr(qcExecuted, bdsInfo.MP[FREQ_INDEX_BDS_B2],3));
        sheet->write(row,ExcelColumnBDSMPB3,getDataStr(qcExecuted, bdsInfo.MP[FREQ_INDEX_BDS_B3],3));
        sheet->write(row,ExcelColumnBDSMPB1C,getDataStr(qcExecuted, bdsInfo.MP[FREQ_INDEX_BDS_B1C],3));
        sheet->write(row,ExcelColumnBDSMPB2A,getDataStr(qcExecuted, bdsInfo.MP[FREQ_INDEX_BDS_B2A],3));
        sheet->write(row,ExcelColumnBDSMPB2B,getDataStr(qcExecuted, bdsInfo.MP[FREQ_INDEX_BDS_B2B],3));
        sheet->write(row,ExcelColumnBDSSNRB1,getDataStr(qcExecuted, bdsInfo.SNR[FREQ_INDEX_BDS_B1],1));
        sheet->write(row,ExcelColumnBDSSNRB2,getDataStr(qcExecuted, bdsInfo.SNR[FREQ_INDEX_BDS_B2],1));
        sheet->write(row,ExcelColumnBDSSNRB3,getDataStr(qcExecuted, bdsInfo.SNR[FREQ_INDEX_BDS_B3],1));
        sheet->write(row,ExcelColumnBDSSNRB1C,getDataStr(qcExecuted, bdsInfo.SNR[FREQ_INDEX_BDS_B1C],1));
        sheet->write(row,ExcelColumnBDSSNRB2A,getDataStr(qcExecuted, bdsInfo.SNR[FREQ_INDEX_BDS_B2A],1));
        sheet->write(row,ExcelColumnBDSSNRB2B,getDataStr(qcExecuted, bdsInfo.SNR[FREQ_INDEX_BDS_B2B],1));

        //GPS
        auto gpsInfo = qcObject->getQCResultManager()->getSysStatisticResult(QCResultManager::QCSysTypeEnum::QCSysGPS);
        sheet->write(row,ExcelColumnGPSUseRate,getDataStr(qcExecuted, gpsInfo.useRate,1,"%"));
        sheet->write(row,ExcelColumnGPSCJR,getDataStr(qcExecuted, gpsInfo.cycleJumpRatio,0));
        sheet->write(row,ExcelColumnGPSMPL1,getDataStr(qcExecuted, gpsInfo.MP[FREQ_INDEX_GPS_L1],3));
        sheet->write(row,ExcelColumnGPSMPL2C,getDataStr(qcExecuted, gpsInfo.MP[FREQ_INDEX_GPS_L2C],3));
        sheet->write(row,ExcelColumnGPSMPL5,getDataStr(qcExecuted, gpsInfo.MP[FREQ_INDEX_GPS_L5],3));
        sheet->write(row,ExcelColumnGPSMPL1C,getDataStr(qcExecuted, gpsInfo.MP[FREQ_INDEX_GPS_L1C],3));
        sheet->write(row,ExcelColumnGPSMPL2P,getDataStr(qcExecuted, gpsInfo.MP[FREQ_INDEX_GPS_L2P],3));
        sheet->write(row,ExcelColumnGPSSNRL1,getDataStr(qcExecuted, gpsInfo.SNR[FREQ_INDEX_GPS_L1],1));
        sheet->write(row,ExcelColumnGPSSNRL2C,getDataStr(qcExecuted, gpsInfo.SNR[FREQ_INDEX_GPS_L2C],1));
        sheet->write(row,ExcelColumnGPSSNRL5,getDataStr(qcExecuted, gpsInfo.SNR[FREQ_INDEX_GPS_L5],1));
        sheet->write(row,ExcelColumnGPSSNRL1C,getDataStr(qcExecuted, gpsInfo.SNR[FREQ_INDEX_GPS_L1C],1));
        sheet->write(row,ExcelColumnGPSSNRL2P,getDataStr(qcExecuted, gpsInfo.SNR[FREQ_INDEX_GPS_L2P],1));

        //GAL
        auto galInfo = qcObject->getQCResultManager()->getSysStatisticResult(QCResultManager::QCSysTypeEnum::QCSysGAL);
        sheet->write(row,ExcelColumnGALUseRate,getDataStr(qcExecuted, galInfo.useRate,1,"%"));
        sheet->write(row,ExcelColumnGALCJR,getDataStr(qcExecuted, galInfo.cycleJumpRatio,0));
        sheet->write(row,ExcelColumnGALMPE1,getDataStr(qcExecuted, galInfo.MP[FREQ_INDEX_GAL_E1],3));
        sheet->write(row,ExcelColumnGALMPE5A,getDataStr(qcExecuted, galInfo.MP[FREQ_INDEX_GAL_E5A],3));
        sheet->write(row,ExcelColumnGALMPE5B,getDataStr(qcExecuted, galInfo.MP[FREQ_INDEX_GAL_E5B],3));
        sheet->write(row,ExcelColumnGALMPE5AB,getDataStr(qcExecuted, galInfo.MP[FREQ_INDEX_GAL_E5AB],3));
        sheet->write(row,ExcelColumnGALMPE6,getDataStr(qcExecuted, galInfo.MP[FREQ_INDEX_GAL_E6],3));
        sheet->write(row,ExcelColumnGALSNRE1,getDataStr(qcExecuted, galInfo.SNR[FREQ_INDEX_GAL_E1],1));
        sheet->write(row,ExcelColumnGALSNRE5A,getDataStr(qcExecuted, galInfo.SNR[FREQ_INDEX_GAL_E5A],1));
        sheet->write(row,ExcelColumnGALSNRE5B,getDataStr(qcExecuted, galInfo.SNR[FREQ_INDEX_GAL_E5B],1));
        sheet->write(row,ExcelColumnGALSNRE5AB,getDataStr(qcExecuted, galInfo.SNR[FREQ_INDEX_GAL_E5AB],1));
        sheet->write(row,ExcelColumnGALSNRE6,getDataStr(qcExecuted, galInfo.SNR[FREQ_INDEX_GAL_E6],1));

        //GLO
        auto gloInfo = qcObject->getQCResultManager()->getSysStatisticResult(QCResultManager::QCSysTypeEnum::QCSysGLO);
        sheet->write(row,ExcelColumnGLOUseRate,getDataStr(qcExecuted, gloInfo.useRate,1,"%"));
        sheet->write(row,ExcelColumnGLOCJR,getDataStr(qcExecuted, gloInfo.cycleJumpRatio,0));
        sheet->write(row,ExcelColumnGLOMPG1,getDataStr(qcExecuted, gloInfo.MP[FREQ_INDEX_GLO_G1],3));
        sheet->write(row,ExcelColumnGLOMPG2,getDataStr(qcExecuted, gloInfo.MP[FREQ_INDEX_GLO_G2],3));
        sheet->write(row,ExcelColumnGLOSNRG1,getDataStr(qcExecuted, gloInfo.SNR[FREQ_INDEX_GLO_G1],1));
        sheet->write(row,ExcelColumnGLOSNRG2,getDataStr(qcExecuted, gloInfo.SNR[FREQ_INDEX_GLO_G2],1));

        //ALL
        auto qzssInfo = qcObject->getQCResultManager()->getSysStatisticResult(QCResultManager::QCSysTypeEnum::QCSysQZSS);
        sheet->write(row,ExcelColumnQZSSUseRate,getDataStr(qcExecuted, qzssInfo.useRate,1,"%"));
        sheet->write(row,ExcelColumnQZSSCJR,getDataStr(qcExecuted, qzssInfo.cycleJumpRatio,0));
        sheet->write(row,ExcelColumnQZSSMPL1,getDataStr(qcExecuted, qzssInfo.MP[FREQ_INDEX_QZSS_L1],3));
        sheet->write(row,ExcelColumnQZSSMPL2,getDataStr(qcExecuted, qzssInfo.MP[FREQ_INDEX_QZSS_L2],3));
        sheet->write(row,ExcelColumnQZSSMPL5,getDataStr(qcExecuted, qzssInfo.MP[FREQ_INDEX_QZSS_L5],3));
        sheet->write(row,ExcelColumnQZSSMPL1CP,getDataStr(qcExecuted, qzssInfo.MP[FREQ_INDEX_QZSS_L1CP],3));
        sheet->write(row,ExcelColumnQZSSMPL6,getDataStr(qcExecuted, qzssInfo.MP[FREQ_INDEX_QZSS_L6],3));
        sheet->write(row,ExcelColumnQZSSSNRL1,getDataStr(qcExecuted, qzssInfo.SNR[FREQ_INDEX_QZSS_L1],1));
        sheet->write(row,ExcelColumnQZSSSNRL2,getDataStr(qcExecuted, qzssInfo.SNR[FREQ_INDEX_QZSS_L2],1));
        sheet->write(row,ExcelColumnQZSSSNRL5,getDataStr(qcExecuted, qzssInfo.SNR[FREQ_INDEX_QZSS_L5],1));
        sheet->write(row,ExcelColumnQZSSSNRL1CP,getDataStr(qcExecuted, qzssInfo.SNR[FREQ_INDEX_QZSS_L1CP],1));
        sheet->write(row,ExcelColumnQZSSSNRL6,getDataStr(qcExecuted, qzssInfo.SNR[FREQ_INDEX_QZSS_L6],1));

        row++;
    }

    // styles
    Format headerFormat;
    headerFormat.setHorizontalAlignment(Format::AlignHCenter);
    headerFormat.setVerticalAlignment(Format::AlignVCenter);
    headerFormat.setBorderStyle(Format::BorderThin);
    headerFormat.setFontBold(true);
    headerFormat.setFontSize(11);
    sheet->setRowFormat(1, 3, headerFormat);

    Format dataFormat;
    dataFormat.setFontSize(11);
    sheet->setRowFormat(4,row,dataFormat);

    return xlsx.saveAs(filePath);
}

QString QCResultExporter::getDataStr(bool executedQC, double value, int prec,const QString &suffix)
{
    if(executedQC && (value > 0))
    {
        return QString::number(value,'f',prec) + suffix;
    }
    return "--";
}
