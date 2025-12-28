/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Plot Module
 * *-
 * @file PlotTimeAxisTicker.h
 * @author CHC
 * @date 2025-09-28
 * @brief custom Axis ticker for time plot
 * 
**************************************************************************/
#ifndef PLOTTIMEAXISTICKER_H
#define PLOTTIMEAXISTICKER_H

#include "QCustomplot/qcustomplot.h"

class PlotTimeAxisTicker : public QCPAxisTicker
{
public:
    PlotTimeAxisTicker();
    virtual void generate(const QCPRange& range, const QLocale& locale, QChar formatChar, int precision, QVector<double>& ticks, QVector<double>* subTicks, QVector<QString>* tickLabels) override;
protected:
    QVector<QString> createLabelVector(const QVector<double>& ticks, const QLocale& locale, QChar formatChar, int precision, double tickStep);
    double getClosestClockTickStep(double tickStep);
};

#endif // PLOTTIMEAXISTICKER_H
