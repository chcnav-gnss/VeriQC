/**********************************************************************//**
 * 		VeriQC
 *
 * 		UI Module
 * *-
 * @file QCExceptionDialog.h
 * @author CHC
 * @date 2026-06-12
 * @brief QC exception detail dialog
 *
**************************************************************************/
#ifndef QCEXCEPTIONDIALOG_H
#define QCEXCEPTIONDIALOG_H

#include <QVector>
#include <QSharedPointer>
#include "Components/MyDialog.h"
#include "VeriQC.h"

class QCObject;

namespace Ui {
class QCExceptionDialog;
}

/** Dialog showing QC exception details after processing */
class QCExceptionDialog : public MyDialog
{
    Q_OBJECT

public:
    explicit QCExceptionDialog(const QVector<QSharedPointer<QCObject>> &qcObjects,
                                QWidget *parent = nullptr);
    ~QCExceptionDialog();

    /** Check whether any QC exception exists across the given objects */
    static bool hasAnyExceptions(const QVector<QSharedPointer<QCObject>> &qcObjects);

private:
    void populateTable(const QVector<QSharedPointer<QCObject>> &qcObjects);
    static QString exceptionCodeToString(int code, const QC_EXCEPTION_INFO_T &info);
    static QString systemIndexToName(int sysIndex);

    Ui::QCExceptionDialog *ui;
};

#endif // QCEXCEPTIONDIALOG_H
