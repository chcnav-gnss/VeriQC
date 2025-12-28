#include "MyWaterProgressBar.h"
#include <QTimer>
#include <QtMath>
#include <QPainter>

MyWaterProgressBar::MyWaterProgressBar(QWidget *parent)
    :QProgressBar(parent)
{
    m_borderWidth = 0;
    m_waterColor.setNamedColor("#FA853A");
    m_backgroundColor.setRgb(255, 255, 255);
    m_borderColor.setRgb(120, 120, 120);
    m_textColor.setRgb(0, 0, 0);
    m_dOffset = 0;

    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);
    connect(m_timer, &QTimer::timeout, this, [=](){
        if (this->isVisible())
        {
            m_dOffset += 0.3;
            if (m_dOffset > 2 * M_PI)
            {
                m_dOffset = 0;
            }
            this->update();
        }
    });
    m_timer->start(50);
}

MyWaterProgressBar::~MyWaterProgressBar()
{
}

void MyWaterProgressBar::paintEvent(QPaintEvent *)
{
    if (!this->isVisible())
    {
        return;
    }
    m_progressValue = this->value() < 0 ? 0 : this->value();

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    drawBackGround(&painter);
    drawWaterWave(&painter);
    drawText(&painter);
}

void MyWaterProgressBar::drawBackGround(QPainter* painter)
{
    int width = this->width();
    int height = this->height();
    if (m_borderWidth > 0)
    {
        int max_diameter = qMin(width, height);
        painter->save();
        painter->setBrush(QBrush(m_borderColor));
        painter->setPen(Qt::NoPen);

        painter->drawEllipse(0, 0, max_diameter, max_diameter);
        painter->restore();
    }
    painter->save();

    int min_diameter = qMin(width, height) - (2 * m_borderWidth);
    painter->setBrush(QBrush(m_backgroundColor));
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(m_borderWidth, m_borderWidth, min_diameter, min_diameter);
    painter->restore();
}

void MyWaterProgressBar::drawWaterWave(QPainter* painter)
{
    int width = this->width();
    int height = this->height();

    int diameter = qMin(width, height) - (2 * m_borderWidth);


    double waveHeight = 0.04*diameter;

    double cycle = 2 * M_PI / diameter;

    double percent = (double)m_progressValue / 100;
    double waterHeight = (1 - percent)*diameter + m_borderWidth;

    painter->save();
    QPainterPath totalPath;

    totalPath.addEllipse(m_borderWidth, m_borderWidth, diameter, diameter);


    QPainterPath water1;
    QPainterPath water2;

    water1.moveTo(m_borderWidth, m_borderWidth + diameter);
    water2.moveTo(m_borderWidth, m_borderWidth + diameter);


    for (int i = m_borderWidth; i <= m_borderWidth + diameter; i++)
    {
        double waterY1 = 0;
        double waterY2 = 0;

        if (m_progressValue == 0 || m_progressValue == 100)
        {
            waterY1 = waterY2 = waterHeight;
        }
        else
        {
            waterY1 = (double)(waveHeight * qSin(cycle * (i - m_borderWidth) - M_PI / 2 + m_dOffset)) + waterHeight;
            waterY2 = (double)(waveHeight * qCos(cycle * (i - m_borderWidth) + m_dOffset)) + waterHeight;
        }
        water1.lineTo(i, waterY1);
        water2.lineTo(i, waterY2);

    }

    water1.lineTo(m_borderWidth + diameter, m_borderWidth + diameter);
    water2.lineTo(m_borderWidth + diameter, m_borderWidth + diameter);

    QPainterPath path;
    QColor waterColor1 = m_waterColor;
    waterColor1.setAlpha(100);
    QColor waterColor2 = m_waterColor;
    waterColor2.setAlpha(200);

    path = totalPath.intersected(water1);
    painter->setBrush(waterColor1);
    painter->setPen(Qt::NoPen);
    painter->drawPath(path);
    painter->restore();
    painter->save();

    path = totalPath.intersected(water2);
    painter->setBrush(waterColor2);
    painter->setPen(Qt::NoPen);
    painter->drawPath(path);

    painter->restore();
}

void MyWaterProgressBar::drawText(QPainter* painter)
{
    painter->save();
    int width = this->width();
    int height = this->height();

    int diameter = qMin(width, height) - (2 * m_borderWidth);
    int fontSize = diameter / 5;
    QFont font = this->font();
    font.setPixelSize(fontSize);
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(m_textColor);
    painter->drawText(QRectF(m_borderWidth, m_borderWidth, diameter, diameter), Qt::AlignCenter, QString("%1%").arg(m_progressValue));
    painter->restore();
}
