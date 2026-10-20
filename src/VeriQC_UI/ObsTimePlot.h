/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Plot Module
 * *-
 * @file ObsTimePlot.h
 * @author CHC
 * @date 2025-09-28
 * @brief time plot for draw OBS sequence
 * 
**************************************************************************/
#ifndef OBSTIMEPLOT_H
#define OBSTIMEPLOT_H

#include <atomic>
#include "Components/MyProgressCustomPlot.h"
#include "QCResultManager.h"
#include "TimePlotGraph.h"

class ObsTimePlot : public MyProgressCustomPlot
{
    Q_OBJECT
public:
    ObsTimePlot(int qcObjectId, QWidget *parent = nullptr);
    QColor getFreqColor(int index);
    void updatePlot();
    void setSysVisible(QCResultManager::QCSysTypeEnum sys, bool visible);
    void setCompactMode(bool compact);
    bool isCompactMode() const { return m_compactMode; }
    bool isUpdating() const { return m_layoutUpdating; }
    void refreshCompactLayout();
protected:
    void doRun() override;
    TimePlotGraph *addObsGraph(QCPAxis *keyAxis, QCPAxis *valueAxis);
    void rescalePlotHeight();
    void adjustAxisMargin(int marginPixels);
    void wheelEvent(QWheelEvent *event) override;
    void onPointHover(const QCPCurveData* hoverPoint, const QPoint & mousePos) override;

private:
    void updatePlotInner();
    void clearCompactGraphs();
    void buildCompactGraphs();
    void applyCompactGeometry(bool rescaleFirst);
    int compactViewportHeight() const;
    int visibleSatCount() const;

    QMap<QCResultManager::QCSysTypeEnum,QVector<TimePlotGraph*>> m_sysGraphsMap;
    QMap<QCResultManager::QCSysTypeEnum,TimePlotGraph*> m_sysCycleJumpGraphMap;
    QHash<QCResultManager::QCSysTypeEnum,bool> m_sysVisibleHash;
    QMap<QCResultManager::QCSysTypeEnum,QHash<unsigned int,unsigned int>> m_sysSatPrnsMap;
    int m_qcObjectId;
    QVector<QCPItemLine*> m_freqBackgroundLines;
    QHash<QString,double> m_yTickLabelHash;
    QFont m_classicTickFont;

    bool m_layoutUpdating = false;
    bool m_pendingRebuild = false;
    bool m_refreshing = false;
    std::atomic<bool> m_compactMode{false};
    QMap<QString,QMap<int,TimePlotGraph*>> m_compactSatGraphs;
    QHash<QString,QCResultManager::QCSysTypeEnum> m_satSysHash;
    QHash<QString,QHash<int,QVector<double>>> m_compactFreqBuffer;
    QHash<QString,QVector<QPair<double,int>>> m_compactSlipBuffer;
};

#endif // OBSTIMEPLOT_H
