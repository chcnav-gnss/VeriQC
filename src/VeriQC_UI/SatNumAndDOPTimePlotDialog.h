/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Plot Module
 * *-
 * @file SatNumAndDOPTimePlotDialog.h
 * @author CHC
 * @date 2025-09-28
 * @brief dialog for controll SatNumAndDOP plot
 * 
**************************************************************************/
#ifndef SATNUMANDDOPTIMEPLOTDIALOG_H
#define SATNUMANDDOPTIMEPLOTDIALOG_H

#include "Components/MyDialog.h"
#include "SatNumAndDOPTimePlot.h"

namespace Ui {
class SatNumAndDOPTimePlotDialog;
}

class SatNumAndDOPTimePlotDialog : public MyDialog
{
    Q_OBJECT

public:
    explicit SatNumAndDOPTimePlotDialog(int qcObjectId,QWidget *parent = nullptr);
    ~SatNumAndDOPTimePlotDialog();
private:
    Ui::SatNumAndDOPTimePlotDialog *ui;
    SatNumAndDOPTimePlot *m_plot;
};

#endif // SATNUMANDDOPTIMEPLOTDIALOG_H
