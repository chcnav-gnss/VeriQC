/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Component Module
 * *-
 * @file MyProgressCustomPlot.h
 * @author CHC
 * @date 2025-09-28
 * @brief time plot that will show progress while loading plot data
 * 
**************************************************************************/
#ifndef MYPROGRESSCUSTOMPLOT_H
#define MYPROGRESSCUSTOMPLOT_H

#include <QRunnable>
#include "../QCustomplot/qcustomplot.h"

class MyProgressCustomPlot : public QCustomPlot, public QRunnable
{
    Q_OBJECT
public:
    MyProgressCustomPlot(QWidget *parent = nullptr);
    void execProgress();
signals:
    void threadStarted();
    void threadFinished();
    void threadProgressRateChanged(int rate);
protected:
    virtual void run() override;
    void onMouseHover();
    virtual void leaveEvent(QEvent*) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void doRun() = 0;
    virtual void onPointHover(const QCPCurveData* hoverPoint, const QPoint& mousePos);
private:
    QTimer* m_hoverTimer = nullptr;
    QPoint m_lastMousePos;
};

#endif // MYPROGRESSCUSTOMPLOT_H
