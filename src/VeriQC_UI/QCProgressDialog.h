/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Module
 * *-
 * @file QCProgressDialog.h
 * @author CHC
 * @date 2025-09-28
 * @brief dialog for showing QC progress
 * 
**************************************************************************/
#ifndef QCPROGRESSDIALOG_H
#define QCPROGRESSDIALOG_H

#include "Components/MyDialog.h"
#include "QCProcessThread.h"

namespace Ui {
class QCProgressDialog;
}

class QCProgressDialog : public MyDialog
{
    Q_OBJECT

public:
    explicit QCProgressDialog(QWidget *parent = nullptr);
    ~QCProgressDialog();
    void execProcess(const QVector<QSharedPointer<QCObject>> &qcObjects, QCProcessThread::ProcessModeEnum mode = QCProcessThread::QCMode);
protected:
    void updateRate(int totalRate, QString currentQCName, int currentQCRate);
    void onError(QString currentQCName, QString errorMsg);
    void onCancel();
private:
    Ui::QCProgressDialog *ui;
    QCProcessThread *m_thread;
};

#endif // QCPROGRESSDIALOG_H
