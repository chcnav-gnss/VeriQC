/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Component Module
 * *-
 * @file MyNoWheelSpinBox.h
 * @author CHC
 * @date 2025-09-28
 * @brief Spin box component of disable wheel event
 * 
**************************************************************************/
#ifndef MYNOWHEELSPINBOX_H
#define MYNOWHEELSPINBOX_H

#include <QSpinBox>

class MyNoWheelSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    MyNoWheelSpinBox(QWidget *parent = nullptr);
protected:
    virtual void wheelEvent(QWheelEvent *event) override;
};

#endif // MYNOWHEELSPINBOX_H
