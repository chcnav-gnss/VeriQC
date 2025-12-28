/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Component Module
 * *-
 * @file MyDialog.h
 * @author CHC
 * @date 2025-09-28
 * @brief Dialog component
 * 
**************************************************************************/
#ifndef MYDIALOG_H
#define MYDIALOG_H

#include <QDialog>

class MyDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MyDialog(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    void setWindowMinimizeMaximizeButtonVisible(bool visible);
};

#endif // MYDIALOG_H
