/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Plot Module
 * *-
 * @file SatNumAndDOPTimePlot.h
 * @author CHC
 * @date 2025-09-28
 * @brief time plot for draw sat number and DOP
 * 
**************************************************************************/
#ifndef SATNUMANDDOPTIMEPLOT_H
#define SATNUMANDDOPTIMEPLOT_H

#include "Components/MyProgressCustomPlot.h"
#include "TimePlotGraph.h"

class SatNumAndDOPTimePlot : public MyProgressCustomPlot
{
    Q_OBJECT
public:
    SatNumAndDOPTimePlot(int qcObjectId,QWidget *parent = nullptr);
    void updatePlot();
    void setPDOPGraphVisible(bool visible);
    void setVDOPGraphVisible(bool visible);
    void setHDOPGraphVisible(bool visible);
    void setTDOPGraphVisible(bool visible);
    static QColor getSatNumGraphColor();
    static QColor getPDOPGraphColor();
    static QColor getVDOPGraphColor();
    static QColor getHDOPGraphColor();
    static QColor getTDOPGraphColor();
protected:
    TimePlotGraph *generateGraph(QCPAxis *keyAxis, QCPAxis *valueAxis, QColor color, bool drawLine);
    void resizeEvent(QResizeEvent *event) override;
    virtual void doRun() override;
private:
    int m_qcObjectId;
    TimePlotGraph *m_satNumGraph;
    TimePlotGraph *m_pDOPGraph;
    TimePlotGraph *m_vDOPGraph;
    TimePlotGraph *m_hDOPGraph;
    TimePlotGraph *m_tDOPGraph;
};

#endif // SATNUMANDDOPTIMEPLOT_H
