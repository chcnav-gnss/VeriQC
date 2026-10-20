#include "ObsTimePlot.h"
#include <QColor>
#include <QFontMetrics>
#include <float.h>
#include <cmath>
#include <algorithm>
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

// all plot systems in enum order GPS..XW1 (y axis sys order)
static QList<QCResultManager::QCSysTypeEnum> allPlotSysEnums()
{
    QList<QCResultManager::QCSysTypeEnum> sysEnums;
    for (int i = (int)QCResultManager::QCSysGPS; i <= (int)QCResultManager::QCSysXW1; ++i)
    {
        sysEnums.append((QCResultManager::QCSysTypeEnum)i);
    }
    return sysEnums;
}

// packed freq sub-lane y inside a satellite row; shared by obs lanes and slip markers so both stay aligned
static double compactLaneY(double satPos, int laneIndex, int laneCount)
{
    return satPos - 0.1 - (laneIndex + 0.5) * 0.8 / qMax(1, laneCount);
}

// compact lanes are faded so the solid red cycle-slip markers stand out on screenshots
static QColor compactLaneColor(QColor color)
{
    color.setAlpha(102);   // ~40% opacity: hue stays identifiable
    return color;
}


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
    m_classicTickFont = this->yAxis->tickLabelFont();   // restore reference for leaving compact mode

    // cycle-slip markers must paint above the frequency lanes: compact per-satellite graphs are
    // created after these system graphs, so without a dedicated layer they would cover the slip dots
    this->addLayer("cycleSlip", this->layer("graphs"), QCustomPlot::limAbove);

    for (QCResultManager::QCSysTypeEnum sys : allPlotSysEnums())
    {
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
        cycleJumpGraph->setLayer("cycleSlip");
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
    if (m_layoutUpdating)
    {
        m_pendingRebuild = true;   // busy (e.g. clicked while loading): queue one full rebuild
        return;
    }
    m_layoutUpdating = true;
    do
    {
        m_pendingRebuild = false;
        updatePlotInner();
    } while (m_pendingRebuild);
    m_layoutUpdating = false;
}

void ObsTimePlot::updatePlotInner()
{
    // match the doRun snapshot: this render pass must lay out the mode the worker filled data for,
    // otherwise toggling compact inside the load progress window yields a blank/mixed frame
    const bool compact = m_compactMode;
    // clear old graphs
    for (QCResultManager::QCSysTypeEnum sys : allPlotSysEnums())
    {
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
    this->clearCompactGraphs();
    m_compactFreqBuffer.clear();
    m_compactSlipBuffer.clear();
    m_satSysHash.clear();

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

    for (QCResultManager::QCSysTypeEnum sys : allPlotSysEnums())
    {
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
    if (compact)
    {
        this->setFixedHeight(this->compactViewportHeight());   // avoid classic tall-height flash while loading
    }
    else
    {
        this->rescalePlotHeight();
    }

    this->execProgress();

    QMap<double,QString> yTickData;
    for(const QString &sat : m_yTickLabelHash.keys())
    {
        yTickData[m_yTickLabelHash[sat]] = sat;
    }
    if (!compact)   // freq lane background lines only in classic layout (sub-pixel clutter when compact)
    {
        double startTime = QCResultManager::qcTime2DateTime(dataList.first().epochObs.time).toMSecsSinceEpoch();
        double endTime = QCResultManager::qcTime2DateTime(dataList.last().epochObs.time).toMSecsSinceEpoch();
        for(const QString &sat : m_yTickLabelHash.keys())
        {
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
    }
    qSharedPointerCast<MyAxisTickerText>(this->yAxis->ticker())->setTicks(yTickData);
    if (compact)
    {
        this->buildCompactGraphs();
        m_compactFreqBuffer.clear();
        m_compactSlipBuffer.clear();
        this->applyCompactGeometry(true);
    }
    else
    {
        this->rescaleAxes(true);
        this->adjustAxisMargin(20);
        this->replot(rpQueuedReplot);
    }
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
    const bool compactMode = m_compactMode;   // snapshot: keep one worker pass in a single mode even if toggled mid-run
    int sysGPSSize = m_sysSatPrnsMap[QCResultManager::QCSysGPS].size();
    int sysBDSSize = m_sysSatPrnsMap[QCResultManager::QCSysBDS].size();
    int sysGLOSize = m_sysSatPrnsMap[QCResultManager::QCSysGLO].size();
    int sysGALSize = m_sysSatPrnsMap[QCResultManager::QCSysGAL].size();
    int sysQZSSSize = m_sysSatPrnsMap[QCResultManager::QCSysQZSS].size();
    int sysXW1Size = m_sysSatPrnsMap[QCResultManager::QCSysXW1].size();

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
                satPos += sysXW1Size    // y axis sys order (top to bottom): GPS,BDS,GLO,GAL,QZSS,XW1
                        + sysQZSSSize
                        + sysGALSize
                        + sysGLOSize
                        + sysBDSSize;
                satPos += sysGPSSize - m_sysSatPrnsMap[QCResultManager::QCSysGPS].value(satData.prn);
                satSys = QCResultManager::QCSysGPS;
                break;
            case QC_SYS_CMP:
                satPos += sysXW1Size
                        + sysQZSSSize
                        + sysGALSize
                        + sysGLOSize;
                satPos += sysBDSSize - m_sysSatPrnsMap[QCResultManager::QCSysBDS].value(satData.prn);
                satSys = QCResultManager::QCSysBDS;
                break;
            case QC_SYS_GLO:
                satPos += sysXW1Size
                        + sysQZSSSize
                        + sysGALSize;
                satPos += sysGLOSize - m_sysSatPrnsMap[QCResultManager::QCSysGLO].value(satData.prn);
                satSys = QCResultManager::QCSysGLO;
                break;
            case QC_SYS_GAL:
                satPos += sysXW1Size
                        + sysQZSSSize;
                satPos += sysGALSize - m_sysSatPrnsMap[QCResultManager::QCSysGAL].value(satData.prn);
                satSys = QCResultManager::QCSysGAL;
                break;
            case QC_SYS_QZS:
                satPos += sysXW1Size;
                satPos += sysQZSSSize - m_sysSatPrnsMap[QCResultManager::QCSysQZSS].value(satData.prn);
                satSys = QCResultManager::QCSysQZSS;
                break;
            case QC_SYS_XW1:
                satPos += 0;
                satPos += sysXW1Size - m_sysSatPrnsMap[QCResultManager::QCSysXW1].value(satData.prn);
                satSys = QCResultManager::QCSysXW1;
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
            if (compactMode)
            {
                m_satSysHash[satName] = satSys;   // lets buildCompactGraphs route slip markers to the owning system graph
            }
            // one doRun pass fills a single mode (snapshot at entry): compact fills plain buffers and the
            // graphs are built on the main thread afterwards; classic fills graphs directly
            for (int i = 0; i < MAX_OBS_PLOT_FREQ_NUM; ++i)
            {
                if(!plotFreqIndexHash[i].contains(satSys))
                {
                    continue;
                }
                int freqIndex = plotFreqIndexHash[i][satSys];
                if(fabs(satData.P[freqIndex]) >= DBL_EPSILON)
                {
                    if (compactMode)
                    {
                        m_compactFreqBuffer[satName][i].append(x);
                    }
                    else
                    {
                        m_sysGraphsMap[satSys][i]->addData(x,satPos - i * 0.1);
                    }
                }
                if(satData.slipFlag[freqIndex])
                {
                    if (compactMode)
                    {
                        m_compactSlipBuffer[satName].append(qMakePair(x,i));
                    }
                    else
                    {
                        m_sysCycleJumpGraphMap[satSys]->addData(x,satPos - i * 0.1);
                    }
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
    qreal dpr = devicePixelRatioF();
    int rowHeight = static_cast<int>(60 * dpr);
    for(QCResultManager::QCSysTypeEnum sys: m_sysVisibleHash.keys())
    {
        if(m_sysVisibleHash[sys])
        {
            height += m_sysSatPrnsMap[sys].size() * rowHeight;
        }
    }
    this->setFixedHeight(height);
}

void ObsTimePlot::setCompactMode(bool compact)
{
    m_compactMode = compact;
    QFont tickFont = m_classicTickFont;
    if (compact)
    {
        tickFont.setPixelSize(11);   // compact tick size of the HTML prototype
    }
    this->yAxis->setTickLabelFont(tickFont);
}

void ObsTimePlot::refreshCompactLayout()
{
    if (!m_compactMode || m_layoutUpdating || m_refreshing)
    {
        return;
    }
    m_refreshing = true;
    this->applyCompactGeometry(false);   // keep current axis range, only recompute height/sizes
    m_refreshing = false;
}

void ObsTimePlot::clearCompactGraphs()
{
    if (m_compactSatGraphs.isEmpty())
    {
        return;
    }
    for (const QString &sat : m_compactSatGraphs.keys())
    {
        for (TimePlotGraph *graph : m_compactSatGraphs[sat])
        {
            this->removePlottable(graph);   // removePlottable deletes the plottable internally
        }
    }
    m_compactSatGraphs.clear();
}

void ObsTimePlot::buildCompactGraphs()
{
    for (const QString &sat : m_compactFreqBuffer.keys())
    {
        if(!m_yTickLabelHash.contains(sat))
        {
            continue;
        }
        double satPos = m_yTickLabelHash[sat];
        QList<int> slotKeys = m_compactFreqBuffer[sat].keys();
        std::sort(slotKeys.begin(), slotKeys.end());
        for (int j = 0; j < slotKeys.size(); ++j)
        {
            int slot = slotKeys.at(j);
            double laneY = compactLaneY(satPos, j, slotKeys.size());
            TimePlotGraph *graph = this->addObsGraph(xAxis,yAxis);
            if(!graph)
            {
                continue;
            }
            graph->setLineStyle(QCPCurve::lsNone);
            graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc,compactLaneColor(this->getFreqColor(slot)),1.0));   // fixed 1px lane dots: final size at creation, no thickening pass (user decision 2026-08-21)
            const QVector<double> &times = m_compactFreqBuffer[sat][slot];
            for(double time : times)
            {
                graph->addData(time,laneY);
            }
            m_compactSatGraphs[sat][slot] = graph;
        }
    }
    for (const QString &sat : m_compactSlipBuffer.keys())
    {
        if(!m_yTickLabelHash.contains(sat) || !m_satSysHash.contains(sat))
        {
            continue;
        }
        double satPos = m_yTickLabelHash[sat];
        QCResultManager::QCSysTypeEnum sys = m_satSysHash[sat];
        QList<int> slotKeys = m_compactFreqBuffer.contains(sat) ? m_compactFreqBuffer[sat].keys() : QList<int>();
        std::sort(slotKeys.begin(), slotKeys.end());
        double centerY = satPos - 0.1 - 0.4;
        for (const QPair<double,int> &slip : m_compactSlipBuffer[sat])
        {
            int j = slotKeys.indexOf(slip.second);
            double laneY = (j >= 0) ? compactLaneY(satPos, j, slotKeys.size()) : centerY;   // slip on slot without obs: row center
            m_sysCycleJumpGraphMap[sys]->addData(slip.first,laneY);
        }
    }
}

void ObsTimePlot::applyCompactGeometry(bool rescaleFirst)
{
    if (rescaleFirst)
    {
        this->rescaleAxes(true);   // x axis bounds (y is fully owned by the explicit compact geometry below)
    }
    int satCount = this->visibleSatCount();
    int targetHeight = this->compactViewportHeight();
    if (satCount > 0 && !m_yTickLabelHash.isEmpty())
    {
        // label extremes: rows occupy [pos-0.9, pos-0.1], labels sit at pos
        QList<double> satPositions = m_yTickLabelHash.values();
        double topPos = *std::max_element(satPositions.begin(), satPositions.end());
        double bottomPos = *std::min_element(satPositions.begin(), satPositions.end());
        double dataUnits = (topPos - bottomPos) + 0.9;   // lane span + top label position

        // row floor = tight ink height of tick label glyphs (sat names are uppercase letter + digits, no descenders)
        // plus the HTML prototype's 20% inter-row gap, so adjacent labels keep a small gap instead of touching
        QFontMetrics tickFontMetrics(this->yAxis->tickLabelFont());
        QRect labelInkRect = tickFontMetrics.tightBoundingRect(QStringLiteral("GCREJ0123456789"));
        double labelInk = labelInkRect.isValid() ? labelInkRect.height() : 1.0 * tickFontMetrics.height();
        double minStride = qMax(6.0, labelInk * 1.25);
        double padPx = labelInk / 2.0 + 2.0;   // half top/bottom label + slack

        double overhead = this->height() - this->yAxis->axisRect()->height();   // chrome px, invariant with widget height
        double targetStride = (targetHeight - overhead - 2 * padPx) / dataUnits;
        double stride = qMax(targetStride, minStride);
        double padUnits = padPx / stride;      // converted at the FINAL stride: stays padPx at any height

        // closed-form geometry: unitPx == stride by construction, padding never scales with window height
        this->yAxis->setRange(bottomPos - 0.9 - padUnits, topPos + padUnits);
        int finalHeight = (int)std::ceil(stride * (dataUnits + 2 * padUnits) + overhead);
        if (targetStride > minStride)
        {
            finalHeight = qMin(finalHeight, targetHeight);   // target-bound: ceil rounding may add 1px and show a needless scrollbar
        }
        if (this->height() != finalHeight)
        {
            this->setFixedHeight(finalHeight);
        }
    }
    else if (this->height() != targetHeight)
    {
        this->setFixedHeight(targetHeight);
    }
    this->replot(rpImmediateRefresh);
}

int ObsTimePlot::compactViewportHeight() const
{
    QWidget *viewport = parentWidget() ? parentWidget()->parentWidget() : nullptr;
    if (viewport)
    {
        return qMax(viewport->height(), 120);
    }
    return 400;
}

int ObsTimePlot::visibleSatCount() const
{
    int count = 0;
    for (QCResultManager::QCSysTypeEnum sys : m_sysVisibleHash.keys())
    {
        if (m_sysVisibleHash[sys])
        {
            count += m_sysSatPrnsMap[sys].size();
        }
    }
    return count;
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

