#include "SkyPlot.h"
#include <QtMath>
#include "SkyPlotSatPolarAxisAngular.h"
#include "QCObjectManager.h"

SkyPlot::SkyPlot(int qcObjectId, QWidget *parent)
    :MyProgressCustomPlot(parent),m_qcObjectId(qcObjectId)
{
    plotLayout()->clear();
    m_angularAxis = new SkyPlotSatPolarAxisAngular(this);
    m_angularAxis->setRangeDrag(false);
    m_angularAxis->setTickLabelMode(QCPPolarAxisAngular::lmUpright);
    m_angularAxis->setSubTicks(false);
    m_angularAxis->setRange(0,360);
    plotLayout()->addElement(0,0,m_angularAxis);

    QSharedPointer<QCPAxisTickerText> ticker(new QCPAxisTickerText);
    ticker->setTicks({0,30,60,90,120,150,180,210,240,270,300,330},{"0","30","60","90","120","150","180","210","240","270","300","330"});
    m_angularAxis->setTicker(ticker);
    m_angularAxis->setTickPen(QPen(QColor(Qt::transparent)));
    QPen angularPen = m_angularAxis->grid()->angularPen();
    angularPen.setColor("#666666");
    m_angularAxis->grid()->setAngularPen(angularPen);
    m_angularAxis->grid()->setRadialPen(angularPen);
    m_angularAxis->grid()->setAngularSubGridPen(angularPen);
    m_angularAxis->setBasePen(angularPen);
    m_angularAxis->setTickLabelColor(QColor("#666666"));

    m_angularAxis->radialAxis()->setTickLabelMode(QCPPolarAxisRadial::lmUpright);
    m_angularAxis->radialAxis()->setTickLabelRotation(0);

    m_angularAxis->radialAxis()->setAngle(0);
    m_angularAxis->radialAxis()->setRange(90,0);
    m_angularAxis->radialAxis()->setRangeReversed(true);
    m_angularAxis->radialAxis()->setSubTicks(false);

    QSharedPointer<QCPAxisTickerText> axisTicker(new QCPAxisTickerText);
    QVector<double> axisTicks{ 0,30,60,90 };
    QVector<QString> axisTickerLabels{ "","30","60","90" };
    axisTicker->setTicks(axisTicks, axisTickerLabels);
    m_angularAxis->radialAxis()->setTicker(axisTicker);
    m_angularAxis->radialAxis()->setTickLabelColor(QColor("#666666"));
    m_angularAxis->radialAxis()->setTickPen(QPen(QColor(Qt::transparent)));
    m_angularAxis->radialAxis()->setBasePen(QPen(QColor(Qt::transparent)));

    //init color gradient   0 = bottom , 1 = top
    m_snrColorGradient.setColorStopAt(1.00, QColor(0xE0, 0x20, 0x20));
    m_snrColorGradient.setColorStopAt(0.83, QColor(0xFA, 0x64, 0x00));
    m_snrColorGradient.setColorStopAt(0.67, QColor(0xF7, 0xB5, 0x00));
    m_snrColorGradient.setColorStopAt(0.50, QColor(0x6D, 0xD4, 0x00));
    m_snrColorGradient.setColorStopAt(0.33, QColor(0x00, 0x91, 0xFF));
    m_snrColorGradient.setColorStopAt(0.17, QColor(0x62, 0x36, 0xFF));
    m_snrColorGradient.setColorStopAt(0.00, QColor(0xB6, 0x20, 0xE0));

    this->setAutoAddPlottableToLegend(false);

    m_sysLayers[QCResultManager::QCSysGPS] = this->createSysLayer("GPSLayer");
    m_sysLayers[QCResultManager::QCSysBDS] = this->createSysLayer("BDSLayer");
    m_sysLayers[QCResultManager::QCSysGLO] = this->createSysLayer("GLOLayer");
    m_sysLayers[QCResultManager::QCSysGAL] = this->createSysLayer("GALLayer");
    m_sysLayers[QCResultManager::QCSysQZSS] = this->createSysLayer("QZSSLayer");

    for (int i = (int)QCResultManager::QCSysGPS; i <= (int)QCResultManager::QCSysQZSS; ++i)
    {
        QCResultManager::QCSysTypeEnum sys = (QCResultManager::QCSysTypeEnum)i;

        SkyPlotPolarSatGraph *satGraph = new SkyPlotPolarSatGraph(m_angularAxis,m_angularAxis->radialAxis());
        satGraph->setLineStyle(SkyPlotPolarSatGraph::lsNone);
        satGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc,1));
        m_sysGraphs[sys] = satGraph;
        satGraph->setLayer(m_sysLayers[sys]);
    }

}

const QCPColorGradient &SkyPlot::getColorGradient() const
{
    return m_snrColorGradient;
}

void SkyPlot::setSysVisible(QCResultManager::QCSysTypeEnum sys, bool visible)
{
    m_sysVisibleMap[sys] = visible;
    m_sysGraphs.value(sys)->setVisible(visible);
    m_sysLayers[sys]->setVisible(visible);
    m_sysLayers[sys]->replot();
}

void SkyPlot::updatePlot()
{
    // clear old graphs
    for(SkyPlotPolarSatGraph *oneSysGraph: m_sysGraphs.values())
    {
        oneSysGraph->clearData();
    }

    QSharedPointer<QCObject> qcObject = QCObjectManager::getInstance()->getQCObjectByID(m_qcObjectId);
    if(!qcObject)
    {
        this->replot(rpQueuedReplot);
        return;
    }
    if(qcObject->getQCResultManager()->getEpochResultList().isEmpty())
    {
        this->replot(rpQueuedReplot);
        return;
    }

    this->execProgress();
    this->replot(rpQueuedReplot);
}

QColor SkyPlot::getSNRColor(double value)
{
    if(value < 20)
    {
        value = 20;
    }
    return m_snrColorGradient.color(value,QCPRange(20,60));
}

void SkyPlot::doRun()
{
    QSharedPointer<QCObject> qcObject = QCObjectManager::getInstance()->getQCObjectByID(m_qcObjectId);
    const QList<QCResultManager::QCEpochResult> & epochDataList = qcObject->getQCResultManager()->getEpochResultList();
    int totalEpochCount = qcObject->getQCResultManager()->getEpochResultList().size();
    double rateFactor = 100.0/totalEpochCount;
    int currentEpochIndex = 0;
    int currentRate = 0;
    for(const QCResultManager::QCEpochResult & epochData : epochDataList)
    {
        currentEpochIndex++;
        int newRate = (int)(currentEpochIndex * rateFactor);
        if(currentRate < newRate)
        {
            currentRate = newRate;
            emit threadProgressRateChanged(newRate);
        }

        for(const QCResultManager::QCEpochSatObsData &satData : epochData.epochObs.data)
        {
            if(!QCResultManager::isSatValid(satData))
            {
                continue;
            }
            QCResultManager::QCSysTypeEnum satSys;
            switch (satData.sys)
            {
            case QC_SYS_GPS:
                satSys = QCResultManager::QCSysGPS;
                break;
            case QC_SYS_CMP:
                satSys = QCResultManager::QCSysBDS;
                break;
            case QC_SYS_GLO:
                satSys = QCResultManager::QCSysGLO;
                break;
            case QC_SYS_GAL:
                satSys = QCResultManager::QCSysGAL;
                break;
            case QC_SYS_QZS:
                satSys = QCResultManager::QCSysQZSS;
                break;
            default:
                continue;
            }

            double x = satData.azel[0] * (180.0/M_PI);  // R2D
            double y = satData.azel[1] * (180.0/M_PI);

            int count = 0;
            double totalSNR = 0;
            for (int i = 0; i < QC_NUM_FREQ; ++i)
            {
                if(satData.SNR[i] > 0)
                {
                    totalSNR += satData.SNR[i];
                    count++;
                }
            }
            double snr = count > 0 ? totalSNR/count : 0;
            Q_ASSERT(m_sysGraphs[satSys]);
            if(!m_sysGraphs[satSys])
            {
                qCritical("CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC");
                continue;
            }
            SkyGraphData pointData;
            pointData.t = currentEpochIndex;
            pointData.key = x;
            pointData.value = y;
            pointData.color = this->getSNRColor(snr);
            pointData.satName = QCResultManager::getSatName(satSys,satData.prn);
            m_sysGraphs[satSys]->addData(pointData);
        }
    }
}

QCPLayer *SkyPlot::createSysLayer(const QString &layerName)
{
    this->addLayer(layerName);
    QCPLayer *newLayer = this->layer(layerName);
    newLayer->setMode(QCPLayer::lmBuffered);
    return newLayer;
}
