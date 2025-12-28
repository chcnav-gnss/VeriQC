/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Component Module
 * *-
 * @file MyAlertMessageBox.h
 * @author CHC
 * @date 2025-09-28
 * @brief Alart message box component
 * 
**************************************************************************/
#ifndef MYALERTMESSAGEBOX_H
#define MYALERTMESSAGEBOX_H

#include <QWidget>
#include <QLabel>
#include <QTimer>

class MyAlertMessageBox : public QWidget
{
    Q_OBJECT
public:
    explicit MyAlertMessageBox(QWidget *parent = nullptr);
    void setText(const QString &text);
protected:
    void showEvent(QShowEvent *) override;
private:
    QLabel *m_label;
    QTimer *m_delayTimer;
};

#endif // MYALERTMESSAGEBOX_H
