#include "MyComboBox.h"
#include <QListView>
#include <QGraphicsDropShadowEffect>

MyComboBox::MyComboBox(QWidget *parent)
    :QComboBox(parent)
{
    this->setView(new QListView(this));
    QFrame *viewContainter = qobject_cast<QFrame*> (this->view()->parent());
    viewContainter->setWindowFlags(viewContainter->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    viewContainter->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setOffset(0,0);
    shadowEffect->setBlurRadius(6);
    shadowEffect->setColor(QColor(0, 0, 0, 25));
    viewContainter->setGraphicsEffect(shadowEffect);
    viewContainter->setStyleSheet("margin:0px 0px 6px 0px;");
}
