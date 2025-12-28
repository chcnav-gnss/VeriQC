/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Component Module
 * *-
 * @file MyMessageBox.h
 * @author CHC
 * @date 2025-09-28
 * @brief Common message box component 
 * 
**************************************************************************/
#ifndef MYMESSAGEBOX_H
#define MYMESSAGEBOX_H

#include <QMessageBox>
#include <QDialog>

class MyMessageBox : public QDialog
{
    Q_OBJECT
public:
    MyMessageBox(QWidget *parent = nullptr);
    static QMessageBox::StandardButton information(QWidget *parent, const QString &title,
         const QString &text, QMessageBox::StandardButtons buttons = QMessageBox::Ok,
         QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);
    static QMessageBox::StandardButton question(QWidget *parent, const QString &title,
         const QString &text, QMessageBox::StandardButtons buttons = QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
         QMessageBox::StandardButton defaultButton = QMessageBox::No);
    static QMessageBox::StandardButton warning(QWidget *parent, const QString &title,
         const QString &text, QMessageBox::StandardButtons buttons = QMessageBox::Ok,
         QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);
    static QMessageBox::StandardButton critical(QWidget *parent, const QString &title,
         const QString &text, QMessageBox::StandardButtons buttons = QMessageBox::Ok,
         QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);
protected:
    static QMessageBox::StandardButton execMessageBox(QWidget *parent,QIcon icon, const QString &title,
         const QString &text, QMessageBox::StandardButtons buttons,
         QMessageBox::StandardButton defaultButton);
};

#endif // MYMESSAGEBOX_H
