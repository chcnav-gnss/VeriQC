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
    enum ParseResultEnum
    {
        ParseOk,
        ParseHelpOrVersion,    /**< -h/-v: exit code must stay 0 */
        ParseError,            /**< bad arguments: exit code 2 */
    };
    /** @brief Parse the console arguments (-q/-r mode, input dir, -o output
     *         dir, -c config file) into @p parseResult; returns ParseOk only
     *         when every argument check passed (help/version and invalid
     *         arguments map to the other results). */
    static ParseResultEnum parseCommandLine(QCoreApplication& app, CommandLineConfigData &parseResult);
    void onProgressRateChanged(int totalRate, QString currentQCName, int currentQCRate);
    void onProcessError(QString currentQCName, QString errorMsg);
};

#endif // COMMANDLINEPROCESSOR_H
