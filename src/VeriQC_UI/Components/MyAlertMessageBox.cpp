#include "MyAlertMessageBox.h"
#include <QHBoxLayout>
#include <QStyle>
#include <QGraphicsDropShadowEffect>

MyAlertMessageBox::MyAlertMessageBox(QWidget *parent) : QWidget(parent)
{
    QWidget *contentWidget = new QWidget(this);
    m_label = new QLabel(contentWidget);
    QLabel *iconLabel = new QLabel(contentWidget);
    iconLabel->setPixmap(style()->standardPixmap(QStyle::SP_MessageBoxInformation));
    iconLabel->setScaledContents(true);
    QHBoxLayout *labelLayout = new QHBoxLayout();
    labelLayout->addWidget(iconLabel);
    labelLayout->addWidget(m_label);
    labelLayout->setContentsMargins(12,12,20,12);
    labelLayout->setSpacing(6);


    contentWidget->setLayout(labelLayout);
    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(6,6,6,6);
    mainLayout->addWidget(contentWidget);

    this->setLayout(mainLayout);

    m_delayTimer = new QTimer(this);
    m_delayTimer->setInterval(3000);
    m_delayTimer->setSingleShot(true);
    connect(m_delayTimer,&QTimer::timeout,this,&MyAlertMessageBox::close);

    this->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setOffset(0,0);
    shadowEffect->setBlurRadius(6);
    shadowEffect->setColor(QColor(0, 0, 0, 100));
    this->setGraphicsEffect(shadowEffect);
    contentWidget->setStyleSheet(".QWidget {background-color:white;color:black;border-radius:3px;}");

}

void MyAlertMessageBox::setText(const QString &text)
{
    m_label->setText(text);
}

void MyAlertMessageBox::showEvent(QShowEvent *)
{
    m_delayTimer->start();
    this->adjustSize();
    if(parentWidget())
    {
        int moveX = parentWidget()->width()/2 - this->width()/2;
        if(moveX > 0)
        {
            this->move(moveX, 0);
        }

    }
}
