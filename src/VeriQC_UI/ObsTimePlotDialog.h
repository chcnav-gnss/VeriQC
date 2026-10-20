/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Plot Module
 * *-
 * @file ObsTimePlotDialog.h
 * @author CHC
 * @date 2025-09-28
 * @brief dialog for controll OBS time plot
 * 
**************************************************************************/
#ifndef OBSTIMEPLOTDIALOG_H
#define OBSTIMEPLOTDIALOG_H

#include "Components/MyDialog.h"
#include "ObsTimePlot.h"

namespace Ui {
class ObsTimePlotDialog;
}

class ObsTimePlotDialog : public MyDialog
{
    Q_OBJECT

public:
    explicit ObsTimePlotDialog(int qcObjectId, QWidget *parent = nullptr);
    ~ObsTimePlotDialog();
protected:
    void onGPSCheckBoxClicked(bool checked);
    void onBDSCheckBoxClicked(bool checked);
    void onGLOCheckBoxClicked(bool checked);
    void onGALCheckBoxClicked(bool checked);
    void onQZSSCheckBoxClicked(bool checked);
    void onXW1CheckBoxClicked(bool checked);
    void onCompactCheckBoxClicked(bool checked);

    QPixmap generateFreqLegendIcon(QColor color);
    QPixmap generateCycleJumpLegendIcon();

    bool eventFilter(QObject *watched, QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void updatePlot();
private:
    Ui::ObsTimePlotDialog *ui;
    ObsTimePlot *m_plot;
    int m_lastViewportHeight = -1;
};

#endif // OBSTIMEPLOTDIALOG_H
