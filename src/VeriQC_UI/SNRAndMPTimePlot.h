/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI plot Module
 * *-
 * @file SNRAndMPTimePlot.h
 * @author CHC
 * @date 2025-09-28
 * @brief time plot for draw SNR and MP
 * 
**************************************************************************/
#ifndef SNRANDMPTIMEPLOT_H
#define SNRANDMPTIMEPLOT_H

#include "Components/MyProgressCustomPlot.h"
#include "QCResultManager.h"
#include "TimePlotGraph.h"

class SNRAndMPTimePlot : public MyProgressCustomPlot
{
    Q_OBJECT
public:
    struct FilterDataStruct
    {
        QCResultManager::QCSysTypeEnum sys; // GNSS system, except QCSysComplex
        unsigned int prn;                   // system sat prn, 0 == all sats
        unsigned int freqIndex;             // sat freq index of result data
    };
public:
    SNRAndMPTimePlot(int qcObjectId,QWidget *parent = nullptr);
    void addFilter(QCResultManager::QCSysTypeEnum sys, unsigned int prn, unsigned int freqIndex);
    void clearFilters();
    void updatePlot();
    static QString getSystemGraphColor(QCResultManager::QCSysTypeEnum sys);
protected:
    void initAxisRect(QCPAxisRect *axisRect, const QString &yAxisLabel, bool isBottomRect);
    void resizeEvent(QResizeEvent *event) override;
    virtual void doRun() override;
    void initSystemGraph(QCResultManager::QCSysTypeEnum sys, QCPAxisRect *rect, QMap<QCResultManager::QCSysTypeEnum, TimePlotGraph *> &graphMap);
private:
    int m_qcObjectId;
    QCPAxisRect *m_snrAxisRect;
    QCPAxisRect *m_mpAxisRect;
    QCPAxisRect *m_eleAxisRect;
    QMap<QCResultManager::QCSysTypeEnum,TimePlotGraph*> m_snrGraphMap;
    QMap<QCResultManager::QCSysTypeEnum,TimePlotGraph*> m_mpGraphMap;
    QMap<QCResultManager::QCSysTypeEnum,TimePlotGraph*> m_eleGraphMap;
    QVector<FilterDataStruct> m_filters;
};

#endif // SNRANDMPTIMEPLOT_H
