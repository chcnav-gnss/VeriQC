/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Module
 * *-
 * @file CommandLineProcessor.h
 * @author CHC
 * @date 2025-09-28
 * @brief process command line application
 * 
**************************************************************************/
#ifndef COMMANDLINEPROCESSOR_H
#define COMMANDLINEPROCESSOR_H

#include <QObject>
#include <QString>
#include <QCoreApplication>
#include "VeriQC.h"

class CommandLineProcessor : public QObject
{
    Q_OBJECT
public:
    enum CommandLineProcessMode
    {
        CommandLineQCMode,
        CommandLineRinexConvertMode
    };

    struct CommandLineConfigData
    {
        QString inputDirPath;
        QString outputDirPath;
        CommandLineProcessMode mode;
        QString configFilePath;
    };
public:
    CommandLineProcessor();
    int exec(QCoreApplication& app);
protected:
    static bool parseCommandLine(QCoreApplication& app, CommandLineConfigData &parseResult);
    void onProgressRateChanged(int totalRate, QString currentQCName, int currentQCRate);
    void onProcessError(QString currentQCName, QString errorMsg);
};

#endif // COMMANDLINEPROCESSOR_H
