#include "MyDialog.h"

MyDialog::MyDialog(QWidget *parent, Qt::WindowFlags f)
    :QDialog(parent,f)
{
    this->setWindowFlag(Qt::WindowMinimizeButtonHint,false);
    this->setWindowFlag(Qt::WindowMaximizeButtonHint,true);
    this->setWindowFlag(Qt::WindowContextHelpButtonHint,false);
}

void MyDialog::setWindowMinimizeMaximizeButtonVisible(bool visible)
{
    this->setWindowFlag(Qt::WindowMinimizeButtonHint,visible);
    this->setWindowFlag(Qt::WindowMaximizeButtonHint,visible);
}
