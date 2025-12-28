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
protected:
    virtual void doRun() override;
    TimePlotGraph *addObsGraph(QCPAxis *keyAxis, QCPAxis *valueAxis);
    void rescalePlotHeight();
    void adjustAxisMargin(int marginPixels);
    void wheelEvent(QWheelEvent *event) override;
    virtual void onPointHover(const QCPCurveData* hoverPoint, const QPoint & mousePos) override;

private:
    QMap<QCResultManager::QCSysTypeEnum,QVector<TimePlotGraph*>> m_sysGraphsMap;
    QMap<QCResultManager::QCSysTypeEnum,TimePlotGraph*> m_sysCycleJumpGraphMap;
    QHash<QCResultManager::QCSysTypeEnum,bool> m_sysVisibleHash;
    QMap<QCResultManager::QCSysTypeEnum,QHash<unsigned int,unsigned int>> m_sysSatPrnsMap;
    int m_qcObjectId;
    QVector<QCPItemLine*> m_freqBackgroundLines;
    QHash<QString,double> m_yTickLabelHash;
};

#endif // OBSTIMEPLOT_H
