#include "MyNoWheelSpinBox.h"
#include <QWheelEvent>

MyNoWheelSpinBox::MyNoWheelSpinBox(QWidget *parent)
    :QSpinBox(parent)
{

}

void MyNoWheelSpinBox::wheelEvent(QWheelEvent *event)
{
    event->ignore();
}
