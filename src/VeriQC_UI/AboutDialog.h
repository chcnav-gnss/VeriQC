/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Module
 * *-
 * @file AboutDialog.h
 * @author CHC
 * @date 2025-09-28
 * @brief About dialog
 * 
**************************************************************************/
#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QLocale>
#include "Components/MyDialog.h"

namespace Ui {
class AboutDialog;
}

class AboutDialog : public MyDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QLocale::Language language, QWidget *parent = nullptr);
    ~AboutDialog();
protected:
    void onCheckUpdate();
    void onLearnMore();
    void onHistoricalVersion();
private:
    Ui::AboutDialog *ui;
    QLocale::Language m_language;
};

#endif // ABOUTDIALOG_H
