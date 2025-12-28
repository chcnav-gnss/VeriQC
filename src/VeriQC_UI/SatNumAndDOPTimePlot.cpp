#include "SatNumAndDOPTimePlot.h"
#include "PlotTimeAxisTicker.h"
#include "QCObjectManager.h"

SatNumAndDOPTimePlot::SatNumAndDOPTimePlot(int qcObjectId, QWidget *parent)
    :MyProgressCustomPlot(parent),m_qcObjectId(qcObjectId)
{
    this->setAutoAddPlottableToLegend(false);
    this->setInteractions(QCP::iRangeZoom);

    this->xAxis2->setVisible(false);

    QPen axisPen;
    axisPen.setColor(QColor("#666666"));
    this->xAxis->setSubTicks(false);
    this->xAxis->setBasePen(axisPen);
    this->xAxis->setTickPen(axisPen);
    this->xAxis->setTickLabelColor(axisPen.color());
    this->xAxis->setTicker(QSharedPointer<QCPAxisTicker>(new PlotTimeAxisTicker()));

    this->yAxis->setSubTicks(false);
    this->yAxis->setBasePen(axisPen);
    this->yAxis->setTickPen(axisPen);
    this->yAxis->setTickLabelColor(axisPen.color());

    this->yAxis2->setSubTicks(false);
    this->yAxis2->setBasePen(axisPen);
    this->yAxis2->setTickPen(axisPen);
    this->yAxis2->setTickLabelColor(axisPen.color());
    this->yAxis2->setVisible(true);

    m_satNumGraph = this->generateGraph(this->xAxis,this->yAxis,this->getSatNumGraphColor(),true);
    m_pDOPGraph = this->generateGraph(this->xAxis,this->yAxis2,this->getPDOPGraphColor(),true);
    m_vDOPGraph = this->generateGraph(this->xAxis,this->yAxis2,this->getVDOPGraphColor(),true);
    m_hDOPGraph = this->generateGraph(this->xAxis,this->yAxis2,this->getHDOPGraphColor(),true);
    m_tDOPGraph = this->generateGraph(this->xAxis,this->yAxis2,this->getTDOPGraphColor(),true);

    this->axisRect()->setRangeZoom(Qt::Orientation::Horizontal);
    QMargins rectMargins = this->axisRect()->minimumMargins();
    rectMargins.setTop(0);
    this->axisRect()->setMinimumMargins(rectMargins);
}

void SatNumAndDOPTimePlot::updatePlot()
{
    // clear old graphs
    m_satNumGraph->data().clear();
    m_pDOPGraph->data().clear();
    m_vDOPGraph->data().clear();
    m_hDOPGraph->data().clear();
    m_tDOPGraph->data().clear();

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

    this->rescaleAxes(true);
    this->yAxis->setRange(this->yAxis->range().center(),this->yAxis->range().size()*1.1,Qt::AlignCenter);
    this->yAxis2->setRange(this->yAxis2->range().center(),this->yAxis2->range().size()*1.1,Qt::AlignCenter);
    this->replot(rpQueuedReplot);
}

void SatNumAndDOPTimePlot::setVDOPGraphVisible(bool visible)
{
    this->m_vDOPGraph->setVisible(visible);
    this->replot(rpQueuedReplot);
}

void SatNumAndDOPTimePlot::setHDOPGraphVisible(bool visible)
{
    this->m_hDOPGraph->setVisible(visible);
    this->replot(rpQueuedReplot);
}

void SatNumAndDOPTimePlot::setTDOPGraphVisible(bool visible)
{
    this->m_tDOPGraph->setVisible(visible);
    this->replot(rpQueuedReplot);
}

QColor SatNumAndDOPTimePlot::getSatNumGraphColor()
{
    return QColor("#FA853A");
}

QColor SatNumAndDOPTimePlot::getVDOPGraphColor()
{
    return QColor("#A741FF");
}

QColor SatNumAndDOPTimePlot::getTDOPGraphColor()
{
    return QColor("#4140F6");
}

TimePlotGraph * SatNumAndDOPTimePlot::generateGraph(QCPAxis* keyAxis, QCPAxis* valueAxis, QColor color,bool drawLine)
{
    TimePlotGraph *graph = new TimePlotGraph(keyAxis,valueAxis);
    graph->setScatterStyle(
                QCPScatterStyle(QCPScatterStyle::ssDisc,color,2));
    if(drawLine)
    {
        graph->setLineStyle(TimePlotGraph::lsLine);
        graph->setPen(QPen(QBrush(Qt::lightGray),0,Qt::DashLine));
    }
    else
    {
        graph->setLineStyle(TimePlotGraph::lsNone);
    }

    return graph;
}

void SatNumAndDOPTimePlot::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    // resize and repaint the buffer:
    setViewport(rect());
    int width = rect().width();
    int minXStep = 200;
    int tickCount = width/minXStep;
    this->xAxis->ticker()->setTickCount(tickCount);
    replot(rpQueuedRefresh); // queued refresh is important here, to prevent painting issues in some contexts (e.g. MDI subwindow)
}

void SatNumAndDOPTimePlot::doRun()
{
    QSharedPointer<QCObject> qcObject = QCObjectManager::getInstance()->getQCObjectByID(m_qcObjectId);
    int totalEpochCount = qcObject->getQCResultManager()->getEpochResultList().size();
    double rateFactor = 100.0/totalEpochCount;
    int currentEpochIndex = 0;
    int currentRate = 0;
    // add new graphs
    for(const QCResultManager::QCEpochResult &epochData : qcObject->getQCResultManager()->getEpochResultList())
    {
        currentEpochIndex++;
        int newRate = (int)(currentEpochIndex * rateFactor);
        if(currentRate < newRate)
        {
            currentRate = newRate;
            emit threadProgressRateChanged(newRate);
        }
        double x = QCResultManager::qcTime2DateTime(epochData.epochSolInfo.Time).toMSecsSinceEpoch();
        m_satNumGraph->addData(x,epochData.epochSolInfo.UsedSatNum);
        m_pDOPGraph->addData(x,epochData.epochSolInfo.Dop[1]);
        m_vDOPGraph->addData(x,epochData.epochSolInfo.Dop[3]);
        m_hDOPGraph->addData(x,epochData.epochSolInfo.Dop[2]);
        m_tDOPGraph->addData(x,epochData.epochSolInfo.Dop[4]);
    }
}


QColor SatNumAndDOPTimePlot::getHDOPGraphColor()
{
    return QColor("#22C587");
}

QColor SatNumAndDOPTimePlot::getPDOPGraphColor()
{
    return QColor("#3B8EFF");
}

void SatNumAndDOPTimePlot::setPDOPGraphVisible(bool visible)
{
    this->m_pDOPGraph->setVisible(visible);
    this->replot(rpQueuedReplot);
}
