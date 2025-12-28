/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI plot Module
 * *-
 * @file SkyPlotDialog.h
 * @author CHC
 * @date 2025-09-28
 * @brief dialog for controll sky plot
 * 
**************************************************************************/
#ifndef SKYPLOTDIALOG_H
#define SKYPLOTDIALOG_H

#include "Components/MyDialog.h"
#include "SkyPlot.h"

namespace Ui {
class SkyPlotDialog;
}

class SkyPlotDialog : public MyDialog
{
    Q_OBJECT

public:
    explicit SkyPlotDialog(int qcObjectId, QWidget *parent = nullptr);
    ~SkyPlotDialog();
protected:
    void onGPSCheckBoxClicked(bool checked);
    void onBDSCheckBoxClicked(bool checked);
    void onGLOCheckBoxClicked(bool checked);
    void onGALCheckBoxClicked(bool checked);
    void onQZSSCheckBoxClicked(bool checked);

    void showEvent(QShowEvent *) override;
    void updateSatCountInfo();
private:
    Ui::SkyPlotDialog *ui;
    SkyPlot *m_plot;
    int m_qcObjectId;
};

#endif // SKYPLOTDIALOG_H
