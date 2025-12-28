/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Component Module
 * *-
 * @file MyWaterProgressBar.h
 * @author CHC
 * @date 2025-09-28
 * @brief water progress bar
 * 
**************************************************************************/
#ifndef MYWATERPROGRESSBAR_H
#define MYWATERPROGRESSBAR_H

#include <QProgressBar>

class MyWaterProgressBar : public QProgressBar
{
    Q_OBJECT
public:
    MyWaterProgressBar(QWidget *parent);
    ~MyWaterProgressBar();

protected:
    void paintEvent(QPaintEvent *);
private:
    void drawBackGround(QPainter* painter);
    void drawWaterWave(QPainter* painter);
    void drawText(QPainter* painter);

private:
    int m_borderWidth;
    int m_progressValue;
    double m_dOffset;
    QColor m_waterColor;
    QColor m_backgroundColor;
    QColor m_borderColor;
    QColor m_textColor;
    QTimer *m_timer;
};

#endif // MYWATERPROGRESSBAR_H
