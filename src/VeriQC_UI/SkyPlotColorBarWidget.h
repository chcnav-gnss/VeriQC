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
};

#endif // SKYPLOTCOLORBARWIDGET_H
