/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Component Module
 * *-
 * @file MyDateTimeEdit.h
 * @author CHC
 * @date 2025-09-28
 * @brief Datatime edit component
 * 
**************************************************************************/
#ifndef MYDATETIMEEDIT_H
#define MYDATETIMEEDIT_H

#include <QDateTimeEdit>

class MyDateTimeEdit : public QDateTimeEdit
{
    Q_OBJECT
public:
    MyDateTimeEdit(QWidget *parent = nullptr);
};

#endif // MYDATETIMEEDIT_H
