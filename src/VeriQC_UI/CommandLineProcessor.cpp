#include "CommandLineProcessor.h"
#include <QCommandLineParser>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include "AppSettingsManagement.h"
#include "QCObjectManager.h"
#include "QCProcessThread.h"
#include "QCResultExporter.h"
#include "PathUtils.h"

namespace
{
/** Process exit codes: 0 = all files succeeded; 1 = some files failed
 *  (execution failure or no valid data); 2 = fatal error (bad arguments,
 *  unreadable/mismatched config, invalid dir, no matching files, summary
 *  export failure). */
const int ExitCodeOK = 0;
const int ExitCodeFileFailures = 1;
const int ExitCodeFatal = 2;

/** @brief First line of the QC config section inside a config file (written
 *  by the QC config dialog); sniffed to validate a '-c' config against '-q'. */
const char GNSS_CFG_SECTION_MARKER[] = "> START_CHCGNSSCFG";
/** @brief First line of the Rinex convert config section inside a config file
 *  (written by the Rinex convert config dialog); sniffed to validate a '-c'
 *  config against '-r'. */
const char RINEX_CFG_SECTION_MARKER[] = "> START_CHCRINEXCFG";

/** @brief Closing line of the QC config section: ReadGNSSCfg and the UI-side
 *  parsers stop here, so text-layer scans must not read past it (a combined
 *  QC+Rinex file keeps its Rinex Sys line, where 'I' is the legal NavIC
 *  letter, after this marker). */
const char GNSS_CFG_SECTION_END_MARKER[] = "> END_CHCGNSSCFG";

/** ReadGNSSCfg()/ReadRinexCfg() return TRUE even for a wrong-typed config
 *  file (only a NULL FILE* fails), so the section marker is sniffed here to
 *  catch a QC/Rinex config mismatch instead of silently running defaults.
 *  OpenFailed is kept apart from MarkerMissing so an unreadable file reports
 *  its real cause instead of a bogus marker mismatch. */
enum ConfigSniffResultEnum
{
    ConfigSniffMarkerFound,
    ConfigSniffMarkerMissing,
    ConfigSniffOpenFailed,
};

/** @brief Check which config section a config file carries (three-state result). */
ConfigSniffResultEnum configFileHasSectionMarker(const QString &configFilePath, const char *marker)
{
    QFile file(configFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return ConfigSniffOpenFailed;
    }
    while (!file.atEnd())
    {
        if (file.readLine().contains(marker))
        {
            return ConfigSniffMarkerFound;
        }
    }
    return ConfigSniffMarkerMissing;
}

/** @brief True when a GNSS (QC) config file's Sys line still carries the
 *  legacy letter 'I' (QC Str2Sys dropped it; XW1 is selected with 'L').
 *
 *  Scans the raw text with the engine's key-segment semantics ('#' starts a
 *  comment, the Sys key must precede the '='), so a commented-out
 *  "# Sys = G,I" line does not trigger. The scan stops at the QC section
 *  end marker like ReadGNSSCfg, so a trailing Rinex section (where 'I' is
 *  the legal NavIC letter) cannot raise a false warning; the key match is
 *  case-sensitive like the engine's strstr while the value check stays
 *  case-insensitive because Str2Sys accepts lowercase letters. Lines before
 *  the section start are still checked: ReadGNSSCfg has no start gate and
 *  really consumes them, so a Sys line placed there does lose its 'I'. The
 *  engine's parsed int mask cannot be used here: it has already silently
 *  discarded the 'I'. */
bool configSysLineHasLegacyLetterI(const QString &configFilePath)
{
    QFile file(configFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }
    while (!file.atEnd())
    {
        const QString line = QString::fromLocal8Bit(file.readLine());
        const int hash = line.indexOf(QLatin1Char('#'));
        const QString effective = (hash >= 0) ? line.left(hash) : line;
        if (effective.contains(GNSS_CFG_SECTION_END_MARKER)) break;  /**< engine: section end stops the parser */
        const int eq = effective.indexOf(QLatin1Char('='));
        if (eq < 0) continue;
        if (!effective.left(eq).contains(QLatin1String("Sys"), Qt::CaseSensitive)) continue;
        if (effective.mid(eq + 1).contains(QLatin1Char('I'), Qt::CaseInsensitive)) return true;
    }
    return false;
}

/** @brief Print the extra usage notes appended to '-h': output dir priority,
 *  accepted data file suffixes and the exit code semantics. */
void printUsageNotes()
{
    fprintf(stdout,
            "\nNotes:\n"
            "  Output dir priority: -o > input dir. The config file OutDir is always overridden in console mode.\n"
            "  Accepted data file suffixes (case-insensitive): Rinex obs (*.o, *.25o, ...), *.rtcm2, *.rtcm3, *.hrc, *.hrcx, *.hcn.\n"
            "  Ephemeris files (*.25n, *.25p, ...) and other files are ignored.\n"
            "  QC config Sys letters: GPS:G SBAS:S GLONASS:R Galileo:E QZSS:J BDS:C XW1:L (the legacy letter I is no longer accepted by the QC engine).\n"
            "  Exit codes: 0 = all files succeeded; 1 = some files failed; 2 = fatal error (arguments/config/dir, no matching files, export failed).\n");
}

/** @brief Print the one-line batch summary (total / succeeded / failed /
 *  elapsed seconds). */
void printBatchStats(int totalFiles, int failedCount, const QElapsedTimer &batchTimer)
{
    fprintf(stdout, "\nBatch finished: %d file(s), %d succeeded, %d failed, elapsed %.1f s\n",
            totalFiles, totalFiles - failedCount, failedCount, batchTimer.elapsed() / 1000.0);
}

/** The "no data file found" message is shared by both modes; keep one copy. */
void printNoMatchingFilesError()
{
    fprintf(stderr, "ERROR: No data file found in the input dir (accepted: Rinex obs *.o, *.<yy>o, *.rtcm2, *.rtcm3, *.hrc, *.hrcx, *.hcn, case-insensitive), use '-h' for help information.\n");
}

/** copyDirPathBytes() leaves just a NUL in the OutDir field when the output
 *  dir cannot be created (mkpath failed); without this check the C layer
 *  silently falls back to the input file directory, so the batch results land
 *  in the wrong place and every convert would be reported as FAILED. */
bool outDirCreationFailed(const char *outDir, const QString &dirPath)
{
    if (dirPath.isEmpty())
    {
        fprintf(stderr, "ERROR: Output dir path is empty\n");
        return true;
    }
    if ('\0' != outDir[0])
    {
        return false;
    }
    fprintf(stderr, "ERROR: Output dir cannot be created (%s)\n", dirPath.toLocal8Bit().constData());
    return true;
}
}

static void consoleProgressBar(const QString &title,double rate,double totalRate)
{
    static bool bFirstRate = true;
    static double PrevRate = 0;
    static QString lastTitle;
    // A batch feeds one title per file; restart the bar for each new file so
    // the stale PrevRate of the previous file cannot swallow all output.
    if (title != lastTitle)
    {
        lastTitle = title;
        bFirstRate = true;
        PrevRate = 0;
    }
    if (bFirstRate)
    {
        bFirstRate = false;
        fprintf(stdout, "%s:\n",title.toLocal8Bit().data());
    }
    if (rate - PrevRate < 0.1)
    {
        return;
    }
    PrevRate = rate;
    static const char* s_pPoint[] = { "  ","\xA8\x87 ","\xA8\x86 ","\xA8\x84 ","\xA8\x82 ","\xA8\x80 "}; // ▏▎▌▋▉
    int iRate = (int)rate;
    int Num0 = (100 - iRate) / 5;
    fprintf(stdout, "\r[");

    while (iRate > 5)
    {
        fprintf(stdout, "%s", s_pPoint[5]);
        iRate -= 5;
    }
    if (iRate)
    {
        fprintf(stdout, "%s", s_pPoint[iRate]);
    }
    for (int i = 0; i < Num0; i++)
    {
        fprintf(stdout, "%s", s_pPoint[0]);
    }

    fprintf(stdout, "] %.1f%%,Total: %.1f%%", rate,totalRate);
    if (rate == 100)
    {
        fprintf(stdout, "\n");
    }
}

CommandLineProcessor::CommandLineProcessor()
{

}

int CommandLineProcessor::exec(QCoreApplication &app)
{
#ifdef CONSOLE
    CommandLineConfigData commandLineConfig;
    const ParseResultEnum parseResult = this->parseCommandLine(app,commandLineConfig);
    if(ParseOk != parseResult)
    {
        return (ParseHelpOrVersion == parseResult) ? ExitCodeOK : ExitCodeFatal;
    }

    QElapsedTimer batchTimer;
    batchTimer.start();

    QCProcessThread processThread;
    connect(&processThread,&QCProcessThread::qcProgressRateChanged,this,&CommandLineProcessor::onProgressRateChanged);
    connect(&processThread,&QCProcessThread::qcProcessError,this,&CommandLineProcessor::onProcessError);

    QEventLoop loop;
    connect(&processThread,&QCProcessThread::finished,&loop,&QEventLoop::quit);

    if(CommandLineQCMode == commandLineConfig.mode)
    {
        CHCQC_GNSSCFG_T gnssCfg = {};
        InitGNSSCfg(&gnssCfg);

        if(!commandLineConfig.configFilePath.isEmpty())
        {
            const ConfigSniffResultEnum sniffResult = configFileHasSectionMarker(commandLineConfig.configFilePath,GNSS_CFG_SECTION_MARKER);
            if(ConfigSniffOpenFailed == sniffResult)
            {
                fprintf(stderr,"ERROR: Read config file failed\n");
                return ExitCodeFatal;
            }
            if(ConfigSniffMarkerMissing == sniffResult)
            {
                fprintf(stderr,"ERROR: Config file contains no QC config section (%s), check the config type against the '-q' mode.\n",GNSS_CFG_SECTION_MARKER);
                return ExitCodeFatal;
            }
            QByteArray cfgPathBytes = PathUtils::toLocalFilePathBytes(commandLineConfig.configFilePath);
            FILE *file = fopen(cfgPathBytes.constData(), "rb");
            if (file == NULL)
            {
                fprintf(stderr,"ERROR: Read config file failed\n");
                return ExitCodeFatal;
            }
            const BOOL readCfgOK = ReadGNSSCfg(file,&gnssCfg);
            fclose(file);
            if(!readCfgOK)
            {
                fprintf(stderr,"ERROR: Read config file failed\n");
                return ExitCodeFatal;
            }
            /**< Warn-only: the engine has already dropped the legacy letter
             *  from its parsed mask; XW1 is selected with 'L' since the
             *  upstream XW1 update. */
            if (configSysLineHasLegacyLetterI(commandLineConfig.configFilePath))
            {
                fprintf(stderr,"WARNING: The QC config Sys line uses the legacy letter I; the engine now selects XW1 with L and ignores I (a Sys line with only I is treated as all systems).\n");
            }
        }
        else
        {
            gnssCfg = QCConfigurator().generateAPIConfig(); //default QC config
            gnssCfg.RawFileDataType = -1;    //auto-detect every supported format (the default path maps None->RINEX only)
        }

        PathUtils::copyDirPathBytes(gnssCfg.OutDir,commandLineConfig.outputDirPath);
        if(outDirCreationFailed(gnssCfg.OutDir,commandLineConfig.outputDirPath))
        {
            return ExitCodeFatal;
        }

        for(const QFileInfo &fileInfo : QDir(commandLineConfig.inputDirPath).entryInfoList(QDir::Files))
        {
            if(!QCConfigurator::isBatchCandidateSuffix(fileInfo.suffix()))
            {
                continue;
            }
            CHCQC_GNSSCFG_T fileCfg = gnssCfg;    //per-file copy: only the input file field differs
            PathUtils::copyFilePathBytes(fileCfg.RoverFile,fileInfo.absoluteFilePath());
            auto newObject = QCObjectManager::getInstance()->addQCObject();
            newObject->getQCConfigurator()->setGNSSAPIConfig(fileCfg,fileInfo.absoluteFilePath());
        }

        const QVector<int> batchObjectIds = QCObjectManager::getInstance()->getAllQCObjectIds();
        if(batchObjectIds.isEmpty())
        {
            printNoMatchingFilesError();
            return ExitCodeFatal;
        }

        processThread.setQCObjects(QCObjectManager::getInstance()->getAllQCObjects());
        processThread.setProcessMode(QCProcessThread::QCMode);
        processThread.start();
        loop.exec();
        processThread.wait();    //explicit join: ::finished already fired, results are final

        int failedCount = 0;
        for(int id : batchObjectIds)
        {
            const auto object = QCObjectManager::getInstance()->getQCObjectByID(id);
            if(!object) { continue; }
            if(!object->isQCRunSucceeded() || QCProcessor::isNoValidDataMsg(object->getDataExceptionMsg()))
            {
                ++failedCount;
            }
        }
        printBatchStats(batchObjectIds.size(),failedCount,batchTimer);

        const QString excelPath = commandLineConfig.outputDirPath + "/QCResult.xlsx";
        if(!QCResultExporter::exportToExcel(excelPath,batchObjectIds))
        {
            fprintf(stderr,"ERROR: Export QC result excel file failed (%s)\n",excelPath.toLocal8Bit().constData());
            return ExitCodeFatal;
        }
        return (failedCount > 0) ? ExitCodeFileFailures : ExitCodeOK;
    }
    else    // Rinex convert
    {
        CHCQC_RINEXCFG_T rinexConvertConfig = {};
        InitRinexCfg(&rinexConvertConfig);

        if(!commandLineConfig.configFilePath.isEmpty())
        {
            const ConfigSniffResultEnum sniffResult = configFileHasSectionMarker(commandLineConfig.configFilePath,RINEX_CFG_SECTION_MARKER);
            if(ConfigSniffOpenFailed == sniffResult)
            {
                fprintf(stderr,"ERROR: Read config file failed\n");
                return ExitCodeFatal;
            }
            if(ConfigSniffMarkerMissing == sniffResult)
            {
                fprintf(stderr,"ERROR: Config file contains no Rinex convert config section (%s), check the config type against the '-r' mode.\n",RINEX_CFG_SECTION_MARKER);
                return ExitCodeFatal;
            }
            QByteArray cfgPathBytes = PathUtils::toLocalFilePathBytes(commandLineConfig.configFilePath);
            FILE *file = fopen(cfgPathBytes.constData(), "rb");
            if (file == NULL)
            {
                fprintf(stderr,"ERROR: Read config file failed\n");
                return ExitCodeFatal;
            }
            const BOOL readCfgOK = ReadRinexCfg(file,&rinexConvertConfig);
            fclose(file);
            if(!readCfgOK)
            {
                fprintf(stderr,"ERROR: Read config file failed\n");
                return ExitCodeFatal;
            }
        }
        else
        {
            rinexConvertConfig = QCConfigurator().generateAPIRinexConvertConfig(AppSettingsManagement::getInstance()->getRinexConvertSettings()); //default rinex convert config
            rinexConvertConfig.DataType = -1;    //mapped per file below (the default path maps None->RINEX only)
        }

        PathUtils::copyDirPathBytes(rinexConvertConfig.OutDir,commandLineConfig.outputDirPath);
        if(outDirCreationFailed(rinexConvertConfig.OutDir,commandLineConfig.outputDirPath))
        {
            return ExitCodeFatal;
        }

        for(const QFileInfo &fileInfo : QDir(commandLineConfig.inputDirPath).entryInfoList(QDir::Files))
        {
            if(!QCConfigurator::isBatchCandidateSuffix(fileInfo.suffix()))
            {
                continue;
            }
            CHCQC_RINEXCFG_T fileCfg = rinexConvertConfig;    //per-file copy: input file + DataType differ
            PathUtils::copyFilePathBytes(fileCfg.RawFile,fileInfo.absoluteFilePath());
            if(-1 == static_cast<int>(fileCfg.DataType))
            {
                /** Convert-mode -1 auto detection only reads the HCN header
                 *  (RTCM2/3, HRCX); map the unambiguous suffixes explicitly. */
                const QCConfigurator::RawFileDataFormatEnum suffixFormat = QCConfigurator::getFormatFromSuffix(fileInfo.suffix());
                if(QCConfigurator::RawFileDataFormatEnum::None != suffixFormat)
                {
                    fileCfg.DataType = static_cast<unsigned int>(QCConfigurator::getAPIRawFileDataType(suffixFormat));
                }
            }
            auto newObject = QCObjectManager::getInstance()->addQCObject();
            newObject->getQCConfigurator()->setRinexConvertAPIConfig(fileCfg,fileInfo.absoluteFilePath());
        }

        const QVector<int> batchObjectIds = QCObjectManager::getInstance()->getAllQCObjectIds();
        if(batchObjectIds.isEmpty())
        {
            printNoMatchingFilesError();
            return ExitCodeFatal;
        }

        processThread.setQCObjects(QCObjectManager::getInstance()->getAllQCObjects());
        processThread.setProcessMode(QCProcessThread::RinexConvertMode);
        processThread.start();
        loop.exec();
        processThread.wait();    //explicit join: ::finished already fired, results are final

        int failedCount = 0;
        for(int id : batchObjectIds)
        {
            const auto object = QCObjectManager::getInstance()->getQCObjectByID(id);
            if(!object) { continue; }
            if(!object->isConvertSucceeded())
            {
                ++failedCount;
            }
        }
        printBatchStats(batchObjectIds.size(),failedCount,batchTimer);

        const QString listPath = commandLineConfig.outputDirPath + "/ConvertResult.xlsx";
        if(!QCResultExporter::exportConvertListToExcel(listPath,batchObjectIds))
        {
            fprintf(stderr,"ERROR: Export convert list excel file failed (%s)\n",listPath.toLocal8Bit().constData());
            return ExitCodeFatal;
        }
        return (failedCount > 0) ? ExitCodeFileFailures : ExitCodeOK;
    }
#endif
    return ExitCodeOK;
}

CommandLineProcessor::ParseResultEnum CommandLineProcessor::parseCommandLine(QCoreApplication &app, CommandLineConfigData &parseResult)
{
    QCommandLineParser parser;
    parser.setApplicationDescription("VeriQC console batch processor: apply one config to every data file in a directory.");
    parser.addPositionalArgument("inputPath","input files dir path");
    QCommandLineOption helpOption = parser.addHelpOption();
    QCommandLineOption versionOption = parser.addVersionOption();
    QCommandLineOption qcModeOption("q","Execute as QC mode (exports QCResult.xlsx summary)");
    parser.addOption(qcModeOption);
    QCommandLineOption rinexConvertModeOption("r","Execute as Rinex convert mode (exports ConvertResult.xlsx manifest)");
    parser.addOption(rinexConvertModeOption);
    QCommandLineOption outputPathOption("o","Output dir path (overrides the config file OutDir; defaults to the input dir)","path");
    parser.addOption(outputPathOption);
    QCommandLineOption configFilePathOption("c","Config file path (QC/Rinex convert config matching the execution mode)","path");
    parser.addOption(configFilePathOption);

    if(!parser.parse(app.arguments()))
    {
        fprintf(stderr,"ERROR: %s, use '-h' for help information.\n",parser.errorText().toLocal8Bit().data());
        return ParseError;
    }
    if(parser.isSet(helpOption))
    {
        fprintf(stdout,"%s",parser.helpText().toLocal8Bit().constData());
        printUsageNotes();
        return ParseHelpOrVersion;
    }
    if(parser.isSet(versionOption))
    {
        parser.showVersion();    //prints the version and exits with code 0
    }
    if(!parser.isSet(qcModeOption) && !parser.isSet(rinexConvertModeOption))
    {
        fprintf(stderr, "ERROR: Please specify execution mode '-q' or '-r', use '-h' for help information.\n");
        return ParseError;
    }
    if(parser.isSet(qcModeOption) && parser.isSet(rinexConvertModeOption))
    {
        fprintf(stderr, "ERROR: Please only specify one execution mode '-q' or '-r', use '-h' for help information.\n");
        return ParseError;
    }
    if(parser.isSet(qcModeOption))
    {
        parseResult.mode = CommandLineQCMode;
    }
    if(parser.isSet(rinexConvertModeOption))
    {
        parseResult.mode = CommandLineRinexConvertMode;
    }
    if(parser.positionalArguments().isEmpty())
    {
        fprintf(stderr, "ERROR: Please specify input files dir path, use '-h' for help information.\n");
        return ParseError;
    }
    if(parser.positionalArguments().size() > 1)
    {
        //warning only: the exit code semantics stay unchanged, extra paths are ignored
        fprintf(stderr, "WARNING: Multiple input paths given, only the first one is processed.\n");
    }
    parseResult.inputDirPath = parser.positionalArguments().at(0);
    QFileInfo inputPathInfo(parseResult.inputDirPath);
    if(!inputPathInfo.exists() || !inputPathInfo.isDir())
    {
        fprintf(stderr, "ERROR: Please specify correct input files dir path, use '-h' for help information.\n");
        return ParseError;
    }
    if(parser.isSet(outputPathOption))
    {
        QFileInfo outDirInfo(parser.value(outputPathOption));
        if(outDirInfo.exists() && outDirInfo.isFile())
        {
            fprintf(stderr, "ERROR: The output dir path exists as a file, use '-h' for help information.\n");
            return ParseError;
        }
        parseResult.outputDirPath = outDirInfo.absoluteFilePath();
    }
    else
    {
        parseResult.outputDirPath = parseResult.inputDirPath;
    }
    if(parser.isSet(configFilePathOption))
    {
        QFileInfo configFilePathInfo(parser.value(configFilePathOption));
        if(!configFilePathInfo.exists() || !configFilePathInfo.isFile())
        {
            fprintf(stderr, "ERROR: Please specify correct config files path, use '-h' for help information.\n");
            return ParseError;
        }
        parseResult.configFilePath = configFilePathInfo.absoluteFilePath();
    }
    return ParseOk;
}

void CommandLineProcessor::onProgressRateChanged(int totalRate, QString currentQCName, int currentQCRate)
{
    consoleProgressBar(currentQCName,currentQCRate,totalRate);
}

void CommandLineProcessor::onProcessError(QString currentQCName, QString errorMsg)
{
    fprintf(stderr,"\nERROR: [%s] %s\n",currentQCName.toLocal8Bit().data(),errorMsg.toLocal8Bit().data());
}
