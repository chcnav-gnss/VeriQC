/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Component Module
 * *-
 * @file MyNoWheelDoubleSpinBox.h
 * @author CHC
 * @date 2025-09-28
 * @brief Double spin box of disable wheel event
 * 
**************************************************************************/
#ifndef MYNOWHEELDOUBLESPINBOX_H
#define MYNOWHEELDOUBLESPINBOX_H

#include <QDoubleSpinBox>

class MyNoWheelDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
public:
    MyNoWheelDoubleSpinBox(QWidget *parent = nullptr);
protected:
    virtual void wheelEvent(QWheelEvent *event) override;
};

#endif // MYNOWHEELDOUBLESPINBOX_H
