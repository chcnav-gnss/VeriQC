#include "SNRAndMPTimePlot.h"
#include <QtMath>
#include "PlotTimeAxisTicker.h"
#include "QCObjectManager.h"

SNRAndMPTimePlot::SNRAndMPTimePlot(int qcObjectId, bool hasEphemeris, QWidget *parent)
    :MyProgressCustomPlot(parent),m_qcObjectId(qcObjectId)
{
    this->setAutoAddPlottableToLegend(false);
    this->setInteractions(QCP::iRangeZoom);
    this->plotLayout()->clear();
    this->plotLayout()->setRowSpacing(0);

    m_snrAxisRect = new QCPAxisRect(this);
    m_mpAxisRect = new QCPAxisRect(this);

    initAxisRect(m_snrAxisRect,"SNR(dBHz)",false);
    initAxisRect(m_mpAxisRect,"Multipath(m)",false);

    this->plotLayout()->addElement(0,0,m_snrAxisRect);
    this->plotLayout()->addElement(1,0,m_mpAxisRect);

    QCPMarginGroup *leftMarginGroup = new QCPMarginGroup(this);
    m_snrAxisRect->setMarginGroup(QCP::msLeft,leftMarginGroup);
    m_mpAxisRect->setMarginGroup(QCP::msLeft,leftMarginGroup);

    connect(m_snrAxisRect->axis(QCPAxis::atBottom), SIGNAL(rangeChanged(QCPRange)), m_mpAxisRect->axis(QCPAxis::atBottom), SLOT(setRange(QCPRange)));
    connect(m_mpAxisRect->axis(QCPAxis::atBottom), SIGNAL(rangeChanged(QCPRange)), m_snrAxisRect->axis(QCPAxis::atBottom), SLOT(setRange(QCPRange)));

    // init graphs
    this->initSystemGraph(QCResultManager::QCSysBDS,m_snrAxisRect,m_snrGraphMap);
    this->initSystemGraph(QCResultManager::QCSysGPS,m_snrAxisRect,m_snrGraphMap);
    this->initSystemGraph(QCResultManager::QCSysGLO,m_snrAxisRect,m_snrGraphMap);
    this->initSystemGraph(QCResultManager::QCSysGAL,m_snrAxisRect,m_snrGraphMap);
    this->initSystemGraph(QCResultManager::QCSysQZSS,m_snrAxisRect,m_snrGraphMap);
    this->initSystemGraph(QCResultManager::QCSysXW1,m_snrAxisRect,m_snrGraphMap);

    this->initSystemGraph(QCResultManager::QCSysBDS,m_mpAxisRect,m_mpGraphMap);
    this->initSystemGraph(QCResultManager::QCSysGPS,m_mpAxisRect,m_mpGraphMap);
    this->initSystemGraph(QCResultManager::QCSysGLO,m_mpAxisRect,m_mpGraphMap);
    this->initSystemGraph(QCResultManager::QCSysGAL,m_mpAxisRect,m_mpGraphMap);
    this->initSystemGraph(QCResultManager::QCSysQZSS,m_mpAxisRect,m_mpGraphMap);
    this->initSystemGraph(QCResultManager::QCSysXW1,m_mpAxisRect,m_mpGraphMap);

    /** elevation subplot exists only when ephemeris is available; no-eph mode omits it */
    if (hasEphemeris)
    {
        m_eleAxisRect = new QCPAxisRect(this);
        initAxisRect(m_eleAxisRect,QString::fromLocal8Bit("Elevation(°)"),true);
        this->plotLayout()->addElement(2,0,m_eleAxisRect);
        m_eleAxisRect->setMarginGroup(QCP::msLeft,leftMarginGroup);
        connect(m_snrAxisRect->axis(QCPAxis::atBottom), SIGNAL(rangeChanged(QCPRange)), m_eleAxisRect->axis(QCPAxis::atBottom), SLOT(setRange(QCPRange)));
        connect(m_mpAxisRect->axis(QCPAxis::atBottom), SIGNAL(rangeChanged(QCPRange)), m_eleAxisRect->axis(QCPAxis::atBottom), SLOT(setRange(QCPRange)));
        connect(m_eleAxisRect->axis(QCPAxis::atBottom), SIGNAL(rangeChanged(QCPRange)), m_snrAxisRect->axis(QCPAxis::atBottom), SLOT(setRange(QCPRange)));
        connect(m_eleAxisRect->axis(QCPAxis::atBottom), SIGNAL(rangeChanged(QCPRange)), m_mpAxisRect->axis(QCPAxis::atBottom), SLOT(setRange(QCPRange)));
        this->initSystemGraph(QCResultManager::QCSysBDS,m_eleAxisRect,m_eleGraphMap);
        this->initSystemGraph(QCResultManager::QCSysGPS,m_eleAxisRect,m_eleGraphMap);
        this->initSystemGraph(QCResultManager::QCSysGLO,m_eleAxisRect,m_eleGraphMap);
        this->initSystemGraph(QCResultManager::QCSysGAL,m_eleAxisRect,m_eleGraphMap);
        this->initSystemGraph(QCResultManager::QCSysQZSS,m_eleAxisRect,m_eleGraphMap);
        this->initSystemGraph(QCResultManager::QCSysXW1,m_eleAxisRect,m_eleGraphMap);
    }
}

void SNRAndMPTimePlot::addFilter(QCResultManager::QCSysTypeEnum sys, unsigned int prn, unsigned int freqIndex)
{
    FilterDataStruct filter;
    filter.sys = sys;
    filter.prn = prn;
    filter.freqIndex = freqIndex;

    m_filters.append(filter);
}

void SNRAndMPTimePlot::clearFilters()
{
    m_filters.clear();
}

void SNRAndMPTimePlot::updatePlot()
{
    // clear old graphs
    for(auto graph: m_snrGraphMap)
    {
        graph->data()->clear();
    }
    for(auto graph: m_mpGraphMap)
    {
        graph->data()->clear();
    }
    for(auto graph: m_eleGraphMap)
    {
        graph->data()->clear();
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

    this->rescaleAxes(true);
    m_snrAxisRect->axis(QCPAxis::atLeft)->setRange(m_snrAxisRect->axis(QCPAxis::atLeft)->range().center(),m_snrAxisRect->axis(QCPAxis::atLeft)->range().size()*1.1,Qt::AlignCenter);
    m_mpAxisRect->axis(QCPAxis::atLeft)->setRange(m_mpAxisRect->axis(QCPAxis::atLeft)->range().center(),m_mpAxisRect->axis(QCPAxis::atLeft)->range().size()*1.1,Qt::AlignCenter);
    if(m_eleAxisRect)
    {
        m_eleAxisRect->axis(QCPAxis::atLeft)->setRange(m_eleAxisRect->axis(QCPAxis::atLeft)->range().center(),m_eleAxisRect->axis(QCPAxis::atLeft)->range().size()*1.1,Qt::AlignCenter);
    }
    this->replot(rpQueuedReplot);
}

QString SNRAndMPTimePlot::getSystemGraphColor(QCResultManager::QCSysTypeEnum sys)
{
    /** delegate to QCResultManager to keep a single color source across plots */
    return QCResultManager::getSystemGraphColor(sys).name();
}

void SNRAndMPTimePlot::initAxisRect(QCPAxisRect *axisRect,const QString &yAxisLabel,bool isBottomRect)
{
    axisRect->setMinimumSize(QSize(100,100));
    QPen axisPen;
    axisPen.setColor(QColor("#666666"));
    axisRect->axis(QCPAxis::atLeft)->setSubTicks(false);
    axisRect->axis(QCPAxis::atLeft)->setBasePen(axisPen);
    axisRect->axis(QCPAxis::atLeft)->setTickPen(axisPen);
    axisRect->axis(QCPAxis::atLeft)->setTickLabelColor(axisPen.color());

    axisRect->axis(QCPAxis::atBottom)->setSubTicks(false);
    axisRect->axis(QCPAxis::atBottom)->setBasePen(axisPen);
    axisRect->axis(QCPAxis::atBottom)->setTickPen(axisPen);
    QMargins rectMargins = axisRect->minimumMargins();
    if(isBottomRect)
    {
        axisRect->axis(QCPAxis::atBottom)->setTickLabelColor(axisPen.color());
        axisRect->axis(QCPAxis::atBottom)->setTicker(QSharedPointer<QCPAxisTicker>(new PlotTimeAxisTicker()));
        rectMargins.setTop(0);
    }
    else
    {
        axisRect->axis(QCPAxis::atBottom)->setTickLabels(false);   /**< only the bottom time axis has label */
        axisRect->axis(QCPAxis::atBottom)->setTicker(QSharedPointer<QCPAxisTicker>(new PlotTimeAxisTicker()));
        axisRect->axis(QCPAxis::atBottom)->setTickLabelPadding(0);
        axisRect->axis(QCPAxis::atBottom)->setLabelPadding(0);
        axisRect->axis(QCPAxis::atBottom)->setPadding(0);

        rectMargins.setBottom(0);
        rectMargins.setTop(0);
    }
    axisRect->setMinimumMargins(rectMargins);
    axisRect->axis(QCPAxis::atRight)->setVisible(true);
    axisRect->axis(QCPAxis::atRight)->setTickLabels(false);
    axisRect->axis(QCPAxis::atRight)->setTicks(false);

    axisRect->axis(QCPAxis::atTop)->setVisible(true);
    axisRect->axis(QCPAxis::atTop)->setTickLabels(false);
    axisRect->axis(QCPAxis::atTop)->setTicks(false);

    QCPTextElement* yAxisLabelElement = new QCPTextElement(this);
    axisRect->insetLayout()->addElement(yAxisLabelElement, Qt::AlignTop | Qt::AlignLeft);
    yAxisLabelElement->setText(yAxisLabel);
    yAxisLabelElement->setMargins(QMargins(10, 5, 0, 0));

    axisRect->setRangeZoom(Qt::Orientation::Horizontal);
}

void SNRAndMPTimePlot::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    // resize and repaint the buffer:
    setViewport(rect());
    int width = rect().width();
    int minXStep = 200;
    int tickCount = width/minXStep;
    m_snrAxisRect->axis(QCPAxis::atBottom)->ticker()->setTickCount(tickCount);
    m_mpAxisRect->axis(QCPAxis::atBottom)->ticker()->setTickCount(tickCount);
    if(m_eleAxisRect)
    {
        m_eleAxisRect->axis(QCPAxis::atBottom)->ticker()->setTickCount(tickCount);
    }
    replot(rpQueuedRefresh); // queued refresh is important here, to prevent painting issues in some contexts (e.g. MDI subwindow)
}

void SNRAndMPTimePlot::doRun()
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
        double x = QCResultManager::qcTime2DateTime(epochData.epochObs.time).toMSecsSinceEpoch();
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
            case QC_SYS_XW1:
                satSys = QCResultManager::QCSysXW1;
                break;
            default:
                continue;
            }
            bool isFiltered = false;
            int filteredFreqIndex = 0;
            for(const FilterDataStruct &filterData : m_filters)
            {
                if(filterData.sys != satSys)
                {
                    continue;
                }
                if(filterData.prn != 0 && filterData.prn != satData.prn)
                {
                    continue;
                }
                filteredFreqIndex = filterData.freqIndex;
                isFiltered = true;
                break;
            }
            if(!isFiltered)
            {
                continue;
            }
            if(satData.SNR[filteredFreqIndex] > 0)
            {
                m_snrGraphMap[satSys]->addData(x,satData.SNR[filteredFreqIndex]);
            }
            if(satData.curMP[filteredFreqIndex] > -9998)
            {
                m_mpGraphMap[satSys]->addData(x,satData.curMP[filteredFreqIndex]);
            }
            // Single QMap lookup: value() returns nullptr for missing keys,
            // so the isEmpty() guard was redundant. Cache the pointer to
            // avoid a second lookup via operator[] on the addData path.
            if(auto *eleGraph = m_eleGraphMap.value(satSys))
            {
                eleGraph->addData(x,satData.azel[1] * (180.0/M_PI));
            }
        }
    }
}

void SNRAndMPTimePlot::initSystemGraph(QCResultManager::QCSysTypeEnum sys,QCPAxisRect *rect,QMap<QCResultManager::QCSysTypeEnum,TimePlotGraph*> &graphMap)
{
    TimePlotGraph *graph = new TimePlotGraph(rect->axis(QCPAxis::atBottom),rect->axis(QCPAxis::atLeft));
    graph->setPen(QPen(QCResultManager::getSystemGraphColor(sys)));
    graphMap.insert(sys,graph);
}
