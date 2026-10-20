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

    ExcelColumnXW1UseRate,
    ExcelColumnXW1CJR,
    ExcelColumnXW1MPL1L,
    ExcelColumnXW1MPL1H,
    ExcelColumnXW1MPL2L,
    ExcelColumnXW1MPL2H,
    ExcelColumnXW1SNRL1L,
    ExcelColumnXW1SNRL1H,
    ExcelColumnXW1SNRL2L,
    ExcelColumnXW1SNRL2H,

    ExcelColumnStatus,
    ExcelColumnReason,
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

    //XW1 header
    sheet->write(1, ExcelColumnXW1UseRate, QObject::tr("XW1"));
    sheet->mergeCells(CellRange(1,ExcelColumnXW1UseRate,1,ExcelColumnXW1SNRL2H));
    sheet->write(2, ExcelColumnXW1UseRate, QObject::tr("Use Rate"));
    sheet->mergeCells(CellRange(2,ExcelColumnXW1UseRate,3,ExcelColumnXW1UseRate));
    sheet->write(2, ExcelColumnXW1CJR, QObject::tr("Cycle Jump Ratio"));
    sheet->mergeCells(CellRange(2,ExcelColumnXW1CJR,3,ExcelColumnXW1CJR));
    sheet->write(2, ExcelColumnXW1MPL1L, QObject::tr("MP(m)"));
    sheet->mergeCells(CellRange(2,ExcelColumnXW1MPL1L,2,ExcelColumnXW1MPL2H));

    sheet->write(3, ExcelColumnXW1MPL1L, QCResultManager::getFreqName(QCResultManager::XW1L1L));
    sheet->write(3, ExcelColumnXW1MPL1H, QCResultManager::getFreqName(QCResultManager::XW1L1H));
    sheet->write(3, ExcelColumnXW1MPL2L, QCResultManager::getFreqName(QCResultManager::XW1L2L));
    sheet->write(3, ExcelColumnXW1MPL2H, QCResultManager::getFreqName(QCResultManager::XW1L2H));

    sheet->write(2, ExcelColumnXW1SNRL1L, QObject::tr("SNR"));
    sheet->mergeCells(CellRange(2,ExcelColumnXW1SNRL1L,2,ExcelColumnXW1SNRL2H));

    sheet->write(3, ExcelColumnXW1SNRL1L, QCResultManager::getFreqName(QCResultManager::XW1L1L));
    sheet->write(3, ExcelColumnXW1SNRL1H, QCResultManager::getFreqName(QCResultManager::XW1L1H));
    sheet->write(3, ExcelColumnXW1SNRL2L, QCResultManager::getFreqName(QCResultManager::XW1L2L));
    sheet->write(3, ExcelColumnXW1SNRL2H, QCResultManager::getFreqName(QCResultManager::XW1L2H));

    //Status/Reason header
    sheet->write(1, ExcelColumnStatus, QObject::tr("Status"));
    sheet->mergeCells(CellRange(1,ExcelColumnStatus,3,ExcelColumnStatus));
    sheet->write(1, ExcelColumnReason, QObject::tr("Reason"));
    sheet->mergeCells(CellRange(1,ExcelColumnReason,3,ExcelColumnReason));
    sheet->setColumnWidth(ExcelColumnStatus,ExcelColumnStatus,20);
    sheet->setColumnWidth(ExcelColumnReason,ExcelColumnReason,45);

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
        /** no-eph: elevation mask is irrelevant; mark cell N/A for batch column alignment */
        const auto &eleCfg = qcObject->getQCConfigurator()->getQCConfig();
        sheet->write(row,ExcelColumnEleDeg,eleCfg.noEphEnable() ? QStringLiteral("N/A") : QString::number(eleCfg.eleDeg));

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

        /**< Per-freq MP/SNR cells read the engine arrays directly via
         *  getSysFreqMPAndSNR (freq-index semantics): the UiComplex-bucket view
         *  returned by getSysStatisticResult reorders the per-freq columns
         *  (e.g. BDS B1 showed B1C, GPS L2C showed L2P) and cannot reach the
         *  PlotExtend-only freqs (GPS L1C / QZSS L1CP / BDS B2B) at all.
         *  The engine writes -1 for freqs without data; getDataStr renders
         *  anything <= 0 as "--". */
        //BDS
        auto bdsInfo = qcObject->getQCResultManager()->getSysStatisticResult(QCResultManager::QCSysTypeEnum::QCSysBDS);
        sheet->write(row,ExcelColumnBDSUseRate,getDataStr(qcExecuted, bdsInfo.useRate,1,"%"));
        sheet->write(row,ExcelColumnBDSCJR,getDataStr(qcExecuted, bdsInfo.cycleJumpRatio,0));
        double freqMP = 0.0;
        double freqSNR = 0.0;
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysBDS, QCResultManager::BDSB1, freqMP, freqSNR);
        sheet->write(row,ExcelColumnBDSMPB1,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnBDSSNRB1,getDataStr(qcExecuted, freqSNR,1));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysBDS, QCResultManager::BDSB2, freqMP, freqSNR);
        sheet->write(row,ExcelColumnBDSMPB2,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnBDSSNRB2,getDataStr(qcExecuted, freqSNR,1));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysBDS, QCResultManager::BDSB3, freqMP, freqSNR);
        sheet->write(row,ExcelColumnBDSMPB3,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnBDSSNRB3,getDataStr(qcExecuted, freqSNR,1));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysBDS, QCResultManager::BDSB1C, freqMP, freqSNR);
        sheet->write(row,ExcelColumnBDSMPB1C,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnBDSSNRB1C,getDataStr(qcExecuted, freqSNR,1));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysBDS, QCResultManager::BDSB2A, freqMP, freqSNR);
        sheet->write(row,ExcelColumnBDSMPB2A,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnBDSSNRB2A,getDataStr(qcExecuted, freqSNR,1));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysBDS, QCResultManager::BDSB2B, freqMP, freqSNR);
        sheet->write(row,ExcelColumnBDSMPB2B,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnBDSSNRB2B,getDataStr(qcExecuted, freqSNR,1));

        //GPS
        auto gpsInfo = qcObject->getQCResultManager()->getSysStatisticResult(QCResultManager::QCSysTypeEnum::QCSysGPS);
        sheet->write(row,ExcelColumnGPSUseRate,getDataStr(qcExecuted, gpsInfo.useRate,1,"%"));
        sheet->write(row,ExcelColumnGPSCJR,getDataStr(qcExecuted, gpsInfo.cycleJumpRatio,0));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysGPS, QCResultManager::GPSL1, freqMP, freqSNR);
        sheet->write(row,ExcelColumnGPSMPL1,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnGPSSNRL1,getDataStr(qcExecuted, freqSNR,1));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysGPS, QCResultManager::GPSL2C, freqMP, freqSNR);
        sheet->write(row,ExcelColumnGPSMPL2C,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnGPSSNRL2C,getDataStr(qcExecuted, freqSNR,1));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysGPS, QCResultManager::GPSL5, freqMP, freqSNR);
        sheet->write(row,ExcelColumnGPSMPL5,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnGPSSNRL5,getDataStr(qcExecuted, freqSNR,1));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysGPS, QCResultManager::GPSL1C, freqMP, freqSNR);
        sheet->write(row,ExcelColumnGPSMPL1C,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnGPSSNRL1C,getDataStr(qcExecuted, freqSNR,1));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysGPS, QCResultManager::GPSL2P, freqMP, freqSNR);
        sheet->write(row,ExcelColumnGPSMPL2P,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnGPSSNRL2P,getDataStr(qcExecuted, freqSNR,1));

        //GAL
        auto galInfo = qcObject->getQCResultManager()->getSysStatisticResult(QCResultManager::QCSysTypeEnum::QCSysGAL);
        sheet->write(row,ExcelColumnGALUseRate,getDataStr(qcExecuted, galInfo.useRate,1,"%"));
        sheet->write(row,ExcelColumnGALCJR,getDataStr(qcExecuted, galInfo.cycleJumpRatio,0));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysGAL, QCResultManager::GALE1, freqMP, freqSNR);
        sheet->write(row,ExcelColumnGALMPE1,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnGALSNRE1,getDataStr(qcExecuted, freqSNR,1));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysGAL, QCResultManager::GALE5A, freqMP, freqSNR);
        sheet->write(row,ExcelColumnGALMPE5A,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnGALSNRE5A,getDataStr(qcExecuted, freqSNR,1));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysGAL, QCResultManager::GALE5B, freqMP, freqSNR);
        sheet->write(row,ExcelColumnGALMPE5B,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnGALSNRE5B,getDataStr(qcExecuted, freqSNR,1));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysGAL, QCResultManager::GALE5AB, freqMP, freqSNR);
        sheet->write(row,ExcelColumnGALMPE5AB,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnGALSNRE5AB,getDataStr(qcExecuted, freqSNR,1));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysGAL, QCResultManager::GALE6, freqMP, freqSNR);
        sheet->write(row,ExcelColumnGALMPE6,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnGALSNRE6,getDataStr(qcExecuted, freqSNR,1));

        //GLO
        auto gloInfo = qcObject->getQCResultManager()->getSysStatisticResult(QCResultManager::QCSysTypeEnum::QCSysGLO);
        sheet->write(row,ExcelColumnGLOUseRate,getDataStr(qcExecuted, gloInfo.useRate,1,"%"));
        sheet->write(row,ExcelColumnGLOCJR,getDataStr(qcExecuted, gloInfo.cycleJumpRatio,0));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysGLO, QCResultManager::GLOG1, freqMP, freqSNR);
        sheet->write(row,ExcelColumnGLOMPG1,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnGLOSNRG1,getDataStr(qcExecuted, freqSNR,1));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysGLO, QCResultManager::GLOG2, freqMP, freqSNR);
        sheet->write(row,ExcelColumnGLOMPG2,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnGLOSNRG2,getDataStr(qcExecuted, freqSNR,1));

        //QZSS
        auto qzssInfo = qcObject->getQCResultManager()->getSysStatisticResult(QCResultManager::QCSysTypeEnum::QCSysQZSS);
        sheet->write(row,ExcelColumnQZSSUseRate,getDataStr(qcExecuted, qzssInfo.useRate,1,"%"));
        sheet->write(row,ExcelColumnQZSSCJR,getDataStr(qcExecuted, qzssInfo.cycleJumpRatio,0));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysQZSS, QCResultManager::QZSSL1, freqMP, freqSNR);
        sheet->write(row,ExcelColumnQZSSMPL1,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnQZSSSNRL1,getDataStr(qcExecuted, freqSNR,1));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysQZSS, QCResultManager::QZSSL2, freqMP, freqSNR);
        sheet->write(row,ExcelColumnQZSSMPL2,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnQZSSSNRL2,getDataStr(qcExecuted, freqSNR,1));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysQZSS, QCResultManager::QZSSL5, freqMP, freqSNR);
        sheet->write(row,ExcelColumnQZSSMPL5,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnQZSSSNRL5,getDataStr(qcExecuted, freqSNR,1));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysQZSS, QCResultManager::QZSSL1CP, freqMP, freqSNR);
        sheet->write(row,ExcelColumnQZSSMPL1CP,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnQZSSSNRL1CP,getDataStr(qcExecuted, freqSNR,1));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysQZSS, QCResultManager::QZSSL6, freqMP, freqSNR);
        sheet->write(row,ExcelColumnQZSSMPL6,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnQZSSSNRL6,getDataStr(qcExecuted, freqSNR,1));

        //XW1
        auto xw1Info = qcObject->getQCResultManager()->getSysStatisticResult(QCResultManager::QCSysTypeEnum::QCSysXW1);
        sheet->write(row,ExcelColumnXW1UseRate,getDataStr(qcExecuted, xw1Info.useRate,1,"%"));
        sheet->write(row,ExcelColumnXW1CJR,getDataStr(qcExecuted, xw1Info.cycleJumpRatio,0));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysXW1, QCResultManager::XW1L1L, freqMP, freqSNR);
        sheet->write(row,ExcelColumnXW1MPL1L,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnXW1SNRL1L,getDataStr(qcExecuted, freqSNR,1));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysXW1, QCResultManager::XW1L1H, freqMP, freqSNR);
        sheet->write(row,ExcelColumnXW1MPL1H,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnXW1SNRL1H,getDataStr(qcExecuted, freqSNR,1));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysXW1, QCResultManager::XW1L2L, freqMP, freqSNR);
        sheet->write(row,ExcelColumnXW1MPL2L,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnXW1SNRL2L,getDataStr(qcExecuted, freqSNR,1));
        qcObject->getQCResultManager()->getSysFreqMPAndSNR(QCResultManager::QCSysTypeEnum::QCSysXW1, QCResultManager::XW1L2H, freqMP, freqSNR);
        sheet->write(row,ExcelColumnXW1MPL2H,getDataStr(qcExecuted, freqMP,3));
        sheet->write(row,ExcelColumnXW1SNRL2H,getDataStr(qcExecuted, freqSNR,1));

        //Status / Reason
        QString statusStr;
        QString reasonStr;
        const QString &dataException = qcObject->getDataExceptionMsg();
        const QCObject::QCResultStatusEnum qcStatus = qcObject->getQCStatus();
        if (QCObject::QCStatusNotExecuted == qcStatus)
        {
            statusStr = QStringLiteral("Not Executed");   // status values stay English (machine-readable); only headers are translated
            reasonStr = qcObject->getLastErrorMsg();
        }
        else if (!qcObject->isQCRunSucceeded())
        {
            statusStr = QStringLiteral("FAILED");
            reasonStr = qcObject->getLastErrorMsg();
        }
        else if (QCProcessor::isNoValidDataMsg(dataException))
        {
            statusStr = QStringLiteral("OK (No Valid Data)");
            reasonStr = dataException;
        }
        else if (QCObject::QCStatusFailed == qcStatus)
        {
            statusStr = QStringLiteral("OK (QC Unqualified)");
            QStringList failedChecks;
            const QCObject::QCCheckStatusInfoStruct &checkInfo = qcObject->getQCCheckStatusInfo();
            if (!checkInfo.useRatePass) { failedChecks << QStringLiteral("UseRate"); }
            if (!checkInfo.cycleJumpRatioPass) { failedChecks << QStringLiteral("CycleJumpRatio"); }
            int mpFailCount = 0;
            int snrFailCount = 0;
            for (int i = 0; i < UI_COMPLEX_MP_MAX_NUM; ++i)
            {
                if (!checkInfo.mpPass[i]) { ++mpFailCount; }
                if (!checkInfo.snrPass[i]) { ++snrFailCount; }
            }
            if (mpFailCount > 0) { failedChecks << QStringLiteral("MP(%1 freqs)").arg(mpFailCount); }
            if (snrFailCount > 0) { failedChecks << QStringLiteral("SNR(%1 freqs)").arg(snrFailCount); }
            reasonStr = QStringLiteral("QC unqualified: %1").arg(failedChecks.join(QStringLiteral(", ")));
        }
        else
        {
            statusStr = QStringLiteral("OK");
            reasonStr = dataException;    // degraded-run note ("Note: ...") or empty
        }
        if (reasonStr.size() > 120)
        {
            reasonStr = reasonStr.left(117) + QStringLiteral("...");
        }
        sheet->write(row,ExcelColumnStatus,statusStr);
        sheet->write(row,ExcelColumnReason,reasonStr);

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

bool QCResultExporter::exportConvertListToExcel(const QString &filePath, const QVector<int> &qcObjects)
{
    using namespace QXlsx;

    enum ConvertColumnEnum
    {
        ConvertColumnFileName = 1,
        ConvertColumnStatus,
        ConvertColumnReason,
        ConvertColumnOutputFiles,
    };

    // create excel
    Document xlsx;
    Worksheet *sheet = xlsx.currentWorksheet();

    // init header
    sheet->write(1, ConvertColumnFileName, QObject::tr("File Name"));
    sheet->write(1, ConvertColumnStatus, QObject::tr("Status"));
    sheet->write(1, ConvertColumnReason, QObject::tr("Reason"));
    sheet->write(1, ConvertColumnOutputFiles, QObject::tr("Output Files"));

    Format headerFormat;
    headerFormat.setHorizontalAlignment(Format::AlignHCenter);
    headerFormat.setVerticalAlignment(Format::AlignVCenter);
    headerFormat.setBorderStyle(Format::BorderThin);
    headerFormat.setFontBold(true);
    headerFormat.setFontSize(11);
    sheet->setRowFormat(1, 1, headerFormat);

    Format wrapFormat;
    wrapFormat.setTextWrap(true);
    wrapFormat.setHorizontalAlignment(Format::AlignLeft);
    wrapFormat.setVerticalAlignment(Format::AlignVCenter);
    wrapFormat.setFontSize(11);

    // fill data
    int row = 2;
    for (int id : qcObjects)
    {
        QSharedPointer<QCObject> qcObject = QCObjectManager::getInstance()->getQCObjectByID(id);
        if (!qcObject) { continue; }

        const bool convertOK = qcObject->isConvertSucceeded();
        const QStringList &outputFiles = qcObject->getConvertOutputFiles();
        sheet->write(row, ConvertColumnFileName, qcObject->getName());
        sheet->write(row, ConvertColumnStatus, convertOK ? QStringLiteral("OK") : QStringLiteral("FAILED"));
        sheet->write(row, ConvertColumnReason,
                     convertOK ? QString() : QStringLiteral("No RINEX observation output generated (see console log)"));
        if (!outputFiles.isEmpty())
        {
            sheet->writeString(row, ConvertColumnOutputFiles, outputFiles.join(QStringLiteral("\n")), wrapFormat);
            sheet->setRowHeight(row, row, 15.0 * outputFiles.size());
        }
        ++row;
    }

    // styles
    Format dataFormat;
    dataFormat.setFontSize(11);
    sheet->setRowFormat(2, row, dataFormat);
    sheet->setColumnWidth(ConvertColumnFileName, ConvertColumnFileName, 34);
    sheet->setColumnWidth(ConvertColumnStatus, ConvertColumnStatus, 10);
    sheet->setColumnWidth(ConvertColumnReason, ConvertColumnReason, 45);
    sheet->setColumnWidth(ConvertColumnOutputFiles, ConvertColumnOutputFiles, 60);

    return xlsx.saveAs(filePath);
}
