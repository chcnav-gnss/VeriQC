/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI plot Module
 * *-
 * @file TimePlotGraph.h
 * @author CHC
 * @date 2025-09-28
 * @brief common graph of time plot
 * 
**************************************************************************/
#ifndef TIMEPLOTGRAPH_H
#define TIMEPLOTGRAPH_H

#include "QCustomplot/qcustomplot.h"

class TimePlotGraph : public QCPCurve
{
    Q_OBJECT
public:
    TimePlotGraph(QCPAxis* keyAxis, QCPAxis* valueAxis);
    virtual ~TimePlotGraph();
protected:
    void getDataScatters(QVector<QCPCurveData>* scatters, const QCPDataRange& dataRange, double scatterWidth);
    virtual void draw(QCPPainter* painter) override;
    void drawGraphScatterPlot(QCPPainter* painter, const QVector<QCPCurveData>& scatters, const QCPScatterStyle& style) const;
    void graphPxielSampling(QVector<QCPCurveData>& scatters)const;
};

#endif // TIMEPLOTGRAPH_H
