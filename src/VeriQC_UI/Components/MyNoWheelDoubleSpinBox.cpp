#include "MyNoWheelDoubleSpinBox.h"
#include <QWheelEvent>

MyNoWheelDoubleSpinBox::MyNoWheelDoubleSpinBox(QWidget *parent)
    :QDoubleSpinBox(parent)
{

}

void MyNoWheelDoubleSpinBox::wheelEvent(QWheelEvent *event)
{
    event->ignore();
}
