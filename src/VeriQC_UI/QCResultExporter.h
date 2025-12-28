/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Report Module
 * *-
 * @file QCResultExporter.h
 * @author CHC
 * @date 2025-09-28
 * @brief export QC result to file
 * 
**************************************************************************/
#ifndef QCRESULTEXPORTER_H
#define QCRESULTEXPORTER_H

#include "QCResultManager.h"

class QCResultExporter
{
public:
    QCResultExporter();
    static bool exportToExcel(const QString& filePath, const QVector<int>& qcObjects);
protected:
    static QString getDataStr(bool executedQC, double value, int prec, const QString &suffix = QString());
};

#endif // QCRESULTEXPORTER_H
