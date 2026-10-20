#include "SkyPlotColorBarWidget.h"
#include "ui_SkyPlotColorBarWidget.h"
#include <QPainter>
#include <QScreen>
#include <QGuiApplication>
#include "QCustomplot/qcustomplot.h"

SkyPlotColorBarWidget::SkyPlotColorBarWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SkyPlotColorBarWidget)
{
    ui->setupUi(this);

    qreal dpr = QGuiApplication::primaryScreen()->devicePixelRatio();
    m_barWidth = static_cast<int>(10 * dpr);
    m_barHeight = static_cast<int>(200 * dpr);
    ui->colorBarLabel->setFixedWidth(m_barWidth);
}

SkyPlotColorBarWidget::~SkyPlotColorBarWidget()
{
    delete ui;
}

void SkyPlotColorBarWidget::setColorGradient(QCPColorGradient colorGradient)
{
    QPixmap pixmap(m_barWidth, m_barHeight);
    QPainter painter(&pixmap);

    QLinearGradient gradient(0, m_barHeight, 0, 0);
    gradient.setColorAt(0.00, colorGradient.color(0,QCPRange(0,1)));
    gradient.setColorAt(0.17, colorGradient.color(0.17,QCPRange(0,1)));
    gradient.setColorAt(0.33, colorGradient.color(0.33,QCPRange(0,1)));
    gradient.setColorAt(0.50, colorGradient.color(0.50,QCPRange(0,1)));
    gradient.setColorAt(0.67, colorGradient.color(0.67,QCPRange(0,1)));
    gradient.setColorAt(0.83, colorGradient.color(0.84,QCPRange(0,1)));
    gradient.setColorAt(1.00, colorGradient.color(1,QCPRange(0,1)));

    painter.fillRect(pixmap.rect(), gradient);


    ui->colorBarLabel->setPixmap(pixmap);
    ui->colorBarLabel->setScaledContents(true);
}
