/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Component Module
 * *-
 * @file MyComboBox.h
 * @author CHC
 * @date 2025-09-28
 * @brief Combo box component
 * 
**************************************************************************/
#ifndef MYCOMBOBOX_H
#define MYCOMBOBOX_H

#include <QComboBox>

class MyComboBox : public QComboBox
{
    Q_OBJECT
public:
    MyComboBox(QWidget *parent = nullptr);
};

#endif // MYCOMBOBOX_H
