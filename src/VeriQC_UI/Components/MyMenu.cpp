#include "MyMenu.h"
#include <QGraphicsDropShadowEffect>


MyMenu::MyMenu(QWidget *parent)
    :QMenu(parent)
{
    this->init();
}

MyMenu::MyMenu(const QString &title, QWidget *parent)
    :QMenu(title,parent)
{
    this->init();
}

void MyMenu::init()
{
    setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setOffset(0,0);
    shadowEffect->setBlurRadius(6);
    shadowEffect->setColor(QColor(0, 0, 0, 25));
    this->setGraphicsEffect(shadowEffect);
    this->setStyleSheet("margin:6px;");
}
