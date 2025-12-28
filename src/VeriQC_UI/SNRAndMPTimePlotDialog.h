/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI plot Module
 * *-
 * @file SNRAndMPTimePlotDialog.h
 * @author CHC
 * @date 2025-09-28
 * @brief dialog for controll SNR and MP plot
 * 
**************************************************************************/
#ifndef SNRANDMPTIMEPLOTDIALOG_H
#define SNRANDMPTIMEPLOTDIALOG_H

#include "Components/MyDialog.h"
#include "SNRAndMPTimePlot.h"

namespace Ui {
class SNRAndMPTimePlotDialog;
}

class SNRAndMPTimePlotDialog : public MyDialog
{
    Q_OBJECT
public:
    struct SatComboBoxItemData
    {
        QCResultManager::QCSysTypeEnum sys;
        unsigned int prn;   // 0 == all sats of the sys
    };

public:
    explicit SNRAndMPTimePlotDialog(int qcObjectId, QWidget *parent = nullptr);
    ~SNRAndMPTimePlotDialog();
protected:
    void initSatComboBox(const QSharedPointer<QCResultManager> &result);
    void satComboBoxAddSysSats(const QSharedPointer<QCResultManager> &result,QCResultManager::QCSysTypeEnum sys);
    void onSatChanged(int index);
    void onFreqChanged();
private:
    Ui::SNRAndMPTimePlotDialog *ui;
    SNRAndMPTimePlot *m_plot;
};

Q_DECLARE_METATYPE(SNRAndMPTimePlotDialog::SatComboBoxItemData)

#endif // SNRANDMPTIMEPLOTDIALOG_H
