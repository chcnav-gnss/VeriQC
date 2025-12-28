/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI plot Module
 * *-
 * @file SkyPlotPolarSatGraph.h
 * @author CHC
 * @date 2025-09-28
 * @brief graph of sky plot
 * 
**************************************************************************/
#ifndef SKYPLOTPOLARSATGRAPH_H
#define SKYPLOTPOLARSATGRAPH_H

#include "QCustomplot/qcustomplot.h"
#include "QCResultManager.h"

class SkyGraphData
{
public:
    SkyGraphData();
    SkyGraphData(double t, double key, double value);

    inline double sortKey() const { return t; }
    inline static SkyGraphData fromSortKey(double sortKey) { return SkyGraphData(sortKey, 0, 0); }
    inline static bool sortKeyIsMainKey() { return false; }

    inline double mainKey() const { return key; }
    inline double mainValue() const { return value; }

    inline QCPRange valueRange() const { return QCPRange(value, value); }

    double t, key, value;
    QColor color;
    QString satName;
};

typedef QCPDataContainer<SkyGraphData> SkyGraphDataContainer;

class SkyPlotPolarSatGraph : public QCPPolarGraph
{
    Q_OBJECT
public:
    struct SkyPlotGraphPointData
    {
        QPoint point;
        QColor color;
    };
public:
    SkyPlotPolarSatGraph(QCPPolarAxisAngular *keyAxis, QCPPolarAxisRadial *valueAxis);
    void addData(const SkyGraphData &data);
    void clearData();

    virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const override;
    virtual QCPRange getKeyRange(bool &foundRange, QCP::SignDomain inSignDomain=QCP::sdBoth) const override;
    virtual QCPRange getValueRange(bool &foundRange, QCP::SignDomain inSignDomain=QCP::sdBoth, const QCPRange &inKeyRange=QCPRange()) const override;

protected:
    void draw(QCPPainter *painter) override;
    void drawPlotScatterPlot(QCPPainter *painter, const QVector<SkyPlotGraphPointData> &scatters, const QCPScatterStyle &style) const;
    void getPlotScatters(QVector<SkyPlotGraphPointData> *scatters, const QCPDataRange &dataRange) const;
    double pointDistance(const QPointF &pixelPoint, SkyGraphDataContainer::const_iterator &closestData) const;
    virtual int dataCount() const override;
    void getVisibleDataBounds(SkyGraphDataContainer::const_iterator &begin, SkyGraphDataContainer::const_iterator &end, const QCPDataRange &rangeRestriction) const;
    void getOptimizedScatterData(QVector<SkyGraphData> *scatterData, SkyGraphDataContainer::const_iterator begin, SkyGraphDataContainer::const_iterator end) const;
private:
    QSharedPointer<SkyGraphDataContainer> m_skyDataContainer;
    QHash<QString,SkyGraphData> m_latestSatPointData;   // key: sat name value: latest sat point data
};

#endif // SKYPLOTPOLARSATGRAPH_H
