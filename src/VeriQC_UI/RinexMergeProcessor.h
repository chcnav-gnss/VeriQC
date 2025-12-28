/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Rinex Convert Module
 * *-
 * @file RinexMergeProcessor.h
 * @author CHC
 * @date 2025-09-28
 * @brief processor for merge Rinex files
 * 
**************************************************************************/
#ifndef RINEXMERGEPROCESSOR_H
#define RINEXMERGEPROCESSOR_H

#include <QThread>
#include <QTimer>

class RinexMergeProcessor : public QThread
{
    Q_OBJECT
public:
    explicit RinexMergeProcessor(QObject *parent = nullptr);
    void startProcess(const QStringList &inputFilePaths, const QString &outputDir);
signals:
    void rateChanged(int rate);
protected:
    virtual void run() override;
    void onUpdateRate();
private:
    QStringList m_inputFilePaths;
    QString m_outputFilePath;
    QTimer *m_progressRateUpdateTimer;
};

#endif // RINEXMERGEPROCESSOR_H
