#include "ObsTimePlot.h"
#include <QColor>
#include <float.h>
#include "PlotTimeAxisTicker.h"
#include "QCObjectManager.h"
#include <QElapsedTimer>
#include <QProgressDialog>
#include "Components/MyWaterProgressBar.h"

#define MAX_OBS_PLOT_FREQ_NUM  ((int)QCResultManager::UiComplexPlotExtend + 1)    // see as plot dialog

class MyAxisTickerText : public QCPAxisTickerText
{
public:
    MyAxisTickerText():QCPAxisTickerText()
    {
        this->setSubTickCount(0);
    }
};



ObsTimePlot::ObsTimePlot(int qcObjectId, QWidget *parent)
    :MyProgressCustomPlot(parent),m_qcObjectId(qcObjectId)
{
    this->setAutoAddPlottableToLegend(false);
    this->setInteractions(QCP::iSelectPlottables | QCP::iRangeZoom);
    this->xAxis->setTicker(QSharedPointer<PlotTimeAxisTicker>(new PlotTimeAxisTicker()));
    QPen tickPen = this->xAxis->tickPen();
    tickPen.setColor("#666666");
    this->xAxis->setTickPen(tickPen);
    this->xAxis->setBasePen(tickPen);
    this->xAxis->setTickLabelColor(QColor("#666666"));
    this->yAxis->setTicker(QSharedPointer<MyAxisTickerText>(new MyAxisTickerText()));
    this->yAxis->setTickLabelColor(QColor("#666666"));
    this->yAxis->setTickPen(tickPen);
    this->yAxis->setBasePen(tickPen);

    for (int i = (int)QCResultManager::QCSysGPS; i <= (int)QCResultManager::QCSysQZSS; ++i)
    {
        QCResultManager::QCSysTypeEnum sys = (QCResultManager::QCSysTypeEnum)i;
        for (int i = 0; i < MAX_OBS_PLOT_FREQ_NUM; ++i)
        {
            TimePlotGraph *freqGraph = this->addObsGraph(xAxis,yAxis);
            freqGraph->setLineStyle(QCPCurve::lsNone);
            freqGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc,this->getFreqColor(i),2));
            m_sysGraphsMap[sys].append(freqGraph);
        }

        TimePlotGraph *cycleJumpGraph = this->addObsGraph(xAxis,yAxis);
        cycleJumpGraph->setLineStyle(QCPCurve::lsNone);
        cycleJumpGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc,"#FF3434",4));
        m_sysCycleJumpGraphMap[sys] = cycleJumpGraph;

        m_sysVisibleHash[sys] = true;
    }
}

QColor ObsTimePlot::getFreqColor(int index)
{
    static const QColor s_FreqColor[8] = {
        QColor("#4140F6"),QColor("#54AB25"),QColor("#A741FF"),
        QColor("#21598D"),QColor("#2BE0E8"),QColor("#EDD820"),
        QColor("#EF9A1E")
    };
    if(index >=0 && index < MAX_OBS_PLOT_FREQ_NUM)
    {
        return s_FreqColor[index];
    }
    return QColor();
}

void ObsTimePlot::updatePlot()
{
    // clear old graphs
    for (int i = (int)QCResultManager::QCSysGPS; i <= (int)QCResultManager::QCSysQZSS; ++i)
    {
        QCResultManager::QCSysTypeEnum sys = (QCResultManager::QCSysTypeEnum)i;
        for (int i = 0; i < MAX_OBS_PLOT_FREQ_NUM; ++i)
        {
            m_sysGraphsMap[sys][i]->data()->clear();
        }
        m_sysCycleJumpGraphMap[sys]->data()->clear();
    }
    for(QCPItemLine *line : m_freqBackgroundLines)
    {
        this->removeItem(line);
    }
    m_freqBackgroundLines.clear();

    QSharedPointer<QCObject> qcObject = QCObjectManager::getInstance()->getQCObjectByID(m_qcObjectId);
    if(!qcObject)
    {
        this->replot(rpQueuedReplot);
        return;
    }
    const QList<QCResultManager::QCEpochResult> & dataList = qcObject->getQCResultManager()->getEpochResultList();
    if(dataList.isEmpty())
    {
        this->replot(rpQueuedReplot);
        return;
    }

    for (int i = (int)QCResultManager::QCSysGPS; i <= (int)QCResultManager::QCSysQZSS; ++i)
    {
        QCResultManager::QCSysTypeEnum sys = (QCResultManager::QCSysTypeEnum)i;
        if(m_sysVisibleHash[sys])
        {
            QList<unsigned int> prns = qcObject->getQCResultManager()->getValidSysSatPrns(sys);
            int index = 0;
            for(unsigned int prn : prns)
            {
                m_sysSatPrnsMap[sys][prn] = index;
                index++;
            }
        }
        else
        {
            m_sysSatPrnsMap[sys].clear();   // if sys is hidden, sys sats area in y axis will ignore
        }
    }
    this->rescalePlotHeight();

    this->execProgress();

    QMap<double,QString> yTickData;
    double startTime = QCResultManager::qcTime2DateTime(dataList.first().epochObs.time).toMSecsSinceEpoch();
    double endTime = QCResultManager::qcTime2DateTime(dataList.last().epochObs.time).toMSecsSinceEpoch();
    for(const QString &sat : m_yTickLabelHash.keys())
    {
        yTickData[m_yTickLabelHash[sat]] = sat;
        for (int i = 0; i < MAX_OBS_PLOT_FREQ_NUM; ++i)
        {
            QCPItemLine *freqBackgroundLine = new QCPItemLine(this);

            freqBackgroundLine->start->setCoords(startTime,m_yTickLabelHash[sat] - i * 0.1);
            freqBackgroundLine->end->setCoords(endTime,m_yTickLabelHash[sat] - i * 0.1);
            freqBackgroundLine->setPen(QPen(QColor("#F7F7F7"),4));
            freqBackgroundLine->setVisible(true);
            freqBackgroundLine->setLayer("grid");
            m_freqBackgroundLines.append(freqBackgroundLine);
        }

    }
    qSharedPointerCast<MyAxisTickerText>(this->yAxis->ticker())->setTicks(yTickData);
    this->rescaleAxes(true);
    this->adjustAxisMargin(20);
    this->replot(rpQueuedReplot);
}

void ObsTimePlot::setSysVisible(QCResultManager::QCSysTypeEnum sys, bool visible)
{
    m_sysVisibleHash[sys] = visible;
    for(TimePlotGraph *graph : m_sysGraphsMap.value(sys))
    {
        graph->setVisible(visible);
    }
    if(m_sysCycleJumpGraphMap.contains(sys))
    {
        m_sysCycleJumpGraphMap[sys]->setVisible(visible);
    }
}

void ObsTimePlot::doRun()
{
    m_yTickLabelHash.clear();
    int sysGPSSize = m_sysSatPrnsMap[QCResultManager::QCSysGPS].size();
    int sysBDSSize = m_sysSatPrnsMap[QCResultManager::QCSysBDS].size();
    int sysGLOSize = m_sysSatPrnsMap[QCResultManager::QCSysGLO].size();
    int sysGALSize = m_sysSatPrnsMap[QCResultManager::QCSysGAL].size();
    int sysQZSSSize = m_sysSatPrnsMap[QCResultManager::QCSysQZSS].size();

    QHash<QCResultManager::QCSysTypeEnum,int> plotFreqIndexHash[MAX_OBS_PLOT_FREQ_NUM] = {};

    for (int i = (int)QCResultManager::UiComplexMP1; i <= (int)QCResultManager::UiComplexPlotExtend; ++i)
    {
        QVector<QCResultManager::FreqTypeEnum> freqs = QCResultManager::getUIComplexFreqTypes((QCResultManager::UiComplexTypeEnum)i);
        for(QCResultManager::FreqTypeEnum freq : freqs)
        {
            QCResultManager::GNSSSysFreqIndexInfoStruct freqInfo = QCResultManager::getFreqIndexInfo(freq);
            plotFreqIndexHash[i][freqInfo.sysType] = freqInfo.freqIndex;
        }
    }

    QSharedPointer<QCObject> qcObject = QCObjectManager::getInstance()->getQCObjectByID(m_qcObjectId);
    int totalEpochCount = qcObject->getQCResultManager()->getEpochResultList().size();
    double rateFactor = 100.0/totalEpochCount;
    int currentEpochIndex = 0;
    int currentRate = 0;
    for(const QCResultManager::QCEpochResult &epochData : qcObject->getQCResultManager()->getEpochResultList())
    {
        currentEpochIndex++;
        int newRate = (int)(currentEpochIndex * rateFactor);
        if(currentRate < newRate)
        {
            currentRate = newRate;
            emit threadProgressRateChanged(newRate);
        }

        double x = QCResultManager::qcTime2DateTime(epochData.epochObs.time).toMSecsSinceEpoch();
        for(const QCResultManager::QCEpochSatObsData &satData : epochData.epochObs.data)
        {
            if(!QCResultManager::isSatValid(satData))
            {
                continue;
            }
            double satPos = 0;
            QCResultManager::QCSysTypeEnum satSys;
            switch (satData.sys)
            {
            case QC_SYS_GPS:
                satPos += sysQZSSSize    // y axis sys order: GPS,BDS,GLO,GAL,QZSS
                        + sysGALSize
                        + sysGLOSize
                        + sysBDSSize;
                satPos += sysGPSSize - m_sysSatPrnsMap[QCResultManager::QCSysGPS].value(satData.prn);
                satSys = QCResultManager::QCSysGPS;
                break;
            case QC_SYS_CMP:
                satPos += sysQZSSSize
                        + sysGALSize
                        + sysGLOSize;
                satPos += sysBDSSize - m_sysSatPrnsMap[QCResultManager::QCSysBDS].value(satData.prn);
                satSys = QCResultManager::QCSysBDS;
                break;
            case QC_SYS_GLO:
                satPos += sysQZSSSize
                        + sysGALSize;
                satPos += sysGLOSize - m_sysSatPrnsMap[QCResultManager::QCSysGLO].value(satData.prn);
                satSys = QCResultManager::QCSysGLO;
                break;
            case QC_SYS_GAL:
                satPos += sysQZSSSize;
                satPos += sysGALSize - m_sysSatPrnsMap[QCResultManager::QCSysGAL].value(satData.prn);
                satSys = QCResultManager::QCSysGAL;
                break;
            case QC_SYS_QZS:
                satPos += 0;
                satPos += sysQZSSSize - m_sysSatPrnsMap[QCResultManager::QCSysQZSS].value(satData.prn);
                satSys = QCResultManager::QCSysQZSS;
                break;
            default:
                continue;
            }
            if(!m_sysVisibleHash[satSys])
            {
                continue;
            }
            QString satName = QCResultManager::getSatName(satSys,satData.prn);
            if(!m_yTickLabelHash.contains(satName))
            {
                m_yTickLabelHash[satName] = satPos;
            }
            for (int i = 0; i < MAX_OBS_PLOT_FREQ_NUM; ++i)
            {
                if(!plotFreqIndexHash[i].contains(satSys))
                {
                    continue;
                }
                if(fabs(satData.P[plotFreqIndexHash[i][satSys]]) >= DBL_EPSILON)
                {
                    double y = satPos - i * 0.1;
                    m_sysGraphsMap[satSys][i]->addData(x,y);
                }
                if(satData.slipFlag[plotFreqIndexHash[i][satSys]])
                {
                    double y = satPos - i * 0.1;
                    m_sysCycleJumpGraphMap[satSys]->addData(x,y);
                }
            }
        }
    }
}


TimePlotGraph* ObsTimePlot::addObsGraph(QCPAxis* keyAxis, QCPAxis* valueAxis)
{
    if (!keyAxis) keyAxis = xAxis;
    if (!valueAxis) valueAxis = yAxis;
    if (!keyAxis || !valueAxis)
    {
        qDebug() << Q_FUNC_INFO << "can't use default QCustomPlot xAxis or yAxis, because at least one is invalid (has been deleted)";
        return nullptr;
    }
    if (keyAxis->parentPlot() != this || valueAxis->parentPlot() != this)
    {
        qDebug() << Q_FUNC_INFO << "passed keyAxis or valueAxis doesn't have this QCustomPlot as parent";
        return nullptr;
    }

    TimePlotGraph* newGraph = new TimePlotGraph(keyAxis, valueAxis);
    newGraph->setAntialiasedScatters(true);
    newGraph->setName(QLatin1String("Graph ") + QString::number(mPlottables.size()));
    return newGraph;
}

void ObsTimePlot::rescalePlotHeight()
{
    int height = 0;
    for(QCResultManager::QCSysTypeEnum sys: m_sysVisibleHash.keys())
    {
        if(m_sysVisibleHash[sys])
        {
            height += m_sysSatPrnsMap[sys].size() * 60;
        }
    }
    this->setFixedHeight(height);
}

void ObsTimePlot::adjustAxisMargin(int marginPixels)
{
    QCPAxis* axes[2] = {
        this->yAxis,
        this->yAxis2,
    };
    for (int i = 0; i < 2; ++i)
    {
        QCPAxis* axis = axes[i];
        if (!axis || !axis->visible()) continue;

        QCPRange range = axis->range();

        double pixelPadding = range.size() * marginPixels / this->height();
        if (axis->scaleType() == QCPAxis::stLinear)
        {
            range.lower -= pixelPadding;
            range.upper += pixelPadding;
        }
        axis->setRange(range);
    }
}

void ObsTimePlot::wheelEvent(QWheelEvent *event)
{
    if(event->modifiers() & Qt::ControlModifier)
    {
        QCustomPlot::wheelEvent(event);
    }
    else
    {
        QWidget *scrollArea = parentWidget()->parentWidget();
        QApplication::sendEvent(scrollArea,event);
    }
}

void ObsTimePlot::onPointHover(const QCPCurveData* hoverPoint, const QPoint& mousePos)
{
    double x = hoverPoint->key;
    QDateTime time = QDateTime::fromMSecsSinceEpoch(x);
    QString tooltipText = time.toString("yyyy-MM-dd HH:mm:ss");
    QToolTip::showText(this->mapToGlobal(mousePos), tooltipText, this);
}

