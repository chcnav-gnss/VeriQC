/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI upgrade Module
 * *-
 * @file CNewVersionDescDialog.h
 * @author CHC
 * @date 2025-09-28
 * @brief New version description dialog when found new version
 * 
**************************************************************************/
#ifndef CNEWVERSIONDESCDIALOG_H
#define CNEWVERSIONDESCDIALOG_H

#include <QDialog>

namespace Ui {
class CNewVersionDescDialog;
}

class CNewVersionDescDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CNewVersionDescDialog(QString newVersion, QString desc, QWidget *parent = nullptr);
    ~CNewVersionDescDialog();

private:
    Ui::CNewVersionDescDialog *ui;
};

#endif // CNEWVERSIONDESCDIALOG_H
