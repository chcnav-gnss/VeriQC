#include "SkyPlotColorBarWidget.h"
#include "ui_SkyPlotColorBarWidget.h"
#include <QPainter>
#include "QCustomplot/qcustomplot.h"

SkyPlotColorBarWidget::SkyPlotColorBarWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SkyPlotColorBarWidget)
{
    ui->setupUi(this);

    ui->colorBarLabel->setFixedWidth(10);
}

SkyPlotColorBarWidget::~SkyPlotColorBarWidget()
{
    delete ui;
}

void SkyPlotColorBarWidget::setColorGradient(QCPColorGradient colorGradient)
{
    QPixmap pixmap(10, 200);
    QPainter painter(&pixmap);

    QLinearGradient gradient(0, 200, 0, 0);
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
