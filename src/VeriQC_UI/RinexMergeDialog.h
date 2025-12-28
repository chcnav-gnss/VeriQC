/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Rinex Convert Module
 * *-
 * @file RinexMergeDialog.h
 * @author CHC
 * @date 2025-09-28
 * @brief dialog for merge multiple Rinex files
 * 
**************************************************************************/
#ifndef RINEXMERGEDIALOG_H
#define RINEXMERGEDIALOG_H

#include "Components/MyDialog.h"

namespace Ui {
class RinexMergeDialog;
}

class RinexMergeDialog : public MyDialog
{
    Q_OBJECT

public:
    explicit RinexMergeDialog(QWidget *parent = nullptr);
    ~RinexMergeDialog();
protected:
    void onAddInputFiles();
    void onOpenOutputDir();
    void onMerge();
    void onInputFilePathsChanged();
private:
    Ui::RinexMergeDialog *ui;
};

#endif // RINEXMERGEDIALOG_H
