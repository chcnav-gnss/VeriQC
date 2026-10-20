/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI plot Module
 * *-
 * @file SkyPlotColorBarWidget.h
 * @author CHC
 * @date 2025-09-28
 * @brief Color bar for sky plot
 * 
**************************************************************************/
#ifndef SKYPLOTCOLORBARWIDGET_H
#define SKYPLOTCOLORBARWIDGET_H

#include <QWidget>
#include "QCustomplot/qcustomplot.h"

namespace Ui {
class SkyPlotColorBarWidget;
}

class SkyPlotColorBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SkyPlotColorBarWidget(QWidget *parent = nullptr);
    ~SkyPlotColorBarWidget();
    void setColorGradient(QCPColorGradient colorGradient);
private:
    Ui::SkyPlotColorBarWidget *ui;
    int m_barWidth;   ///< DPI-scaled color bar width
    int m_barHeight;  ///< DPI-scaled color bar height
};

#endif // SKYPLOTCOLORBARWIDGET_H
