#include "CommandLineProcessor.h"
#include <QCommandLineParser>
#include <QFileInfo>
#include <QDir>
#include <QRegExp>
#include "AppSettingsManagement.h"
#include "QCObjectManager.h"
#include "QCProcessThread.h"
#include "QCResultExporter.h"

static void consoleProgressBar(const QString &title,double rate,double totalRate)
{
    static bool bFirstRate = true;
    static double PrevRate = 0;
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
    if(this->parseCommandLine(app,commandLineConfig))
    {
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
                auto file = fopen(commandLineConfig.configFilePath.toLocal8Bit().data(), "rb+");
                if (file == NULL)
                {
                    fprintf(stderr,"ERROR: Read config file failed\n");
                    return -1;
                }
                ReadGNSSCfg((FILE*)file,&gnssCfg);
                fclose(file);
            }
            else
            {
                gnssCfg = QCConfigurator().generateAPIConfig(); //default QC config
            }

            QString outDir = commandLineConfig.outputDirPath;
            if(!outDir.endsWith("/") && !outDir.endsWith("\\"))
            {
                outDir += QDir::separator();
            }
            memcpy(gnssCfg.OutDir,outDir.toLocal8Bit().data(),outDir.toLocal8Bit().size());


            for(const QFileInfo &fileInfo : QDir(commandLineConfig.inputDirPath).entryInfoList(QDir::Files))
            {
                if(QRegExp("\\d*o").exactMatch(fileInfo.suffix()))
                {
                    QByteArray inputFilePath = fileInfo.absoluteFilePath().toLocal8Bit();
                    memset(gnssCfg.RoverFile,0,sizeof(gnssCfg.RoverFile));
                    memcpy(gnssCfg.RoverFile,inputFilePath.data(),inputFilePath.size());

                    auto newObject = QCObjectManager::getInstance()->addQCObject();
                    newObject->getQCConfigurator()->setGNSSAPIConfig(gnssCfg);
                }
            }
            processThread.setQCObjects(QCObjectManager::getInstance()->getAllQCObjects());
            processThread.setProcessMode(QCProcessThread::QCMode);
            processThread.start();
            loop.exec();

            if(!QCResultExporter::exportToExcel(commandLineConfig.outputDirPath.append("/QCResult.xlsx"),QCObjectManager::getInstance()->getAllQCObjectIds()))
            {
                fprintf(stderr,"ERROR: Export QC result excel file failed\n");
            }
        }
        else    // Rinex convert
        {
            CHCQC_RINEXCFG_T rinexConvertConfig = {};
            InitRinexCfg(&rinexConvertConfig);

            if(!commandLineConfig.configFilePath.isEmpty())
            {
                auto file = fopen(commandLineConfig.configFilePath.toLocal8Bit().data(), "rb+");
                if (file == NULL)
                {
                    fprintf(stderr,"ERROR: Read config file failed\n");
                    return -1;
                }
                ReadRinexCfg((FILE*)file,&rinexConvertConfig);
                fclose(file);
            }
            else
            {
                rinexConvertConfig = QCConfigurator().generateAPIRinexConvertConfig(AppSettingsManagement::getInstance()->getRinexConvertSettings()); //default rinex convert config
            }

            QString outDir = commandLineConfig.outputDirPath;
            if(!outDir.endsWith("/") && !outDir.endsWith("\\"))
            {
                outDir += QDir::separator();
            }
            memcpy(rinexConvertConfig.OutDir,outDir.toLocal8Bit().data(),outDir.toLocal8Bit().size());

            for(const QFileInfo &fileInfo : QDir(commandLineConfig.inputDirPath).entryInfoList(QDir::Files))
            {
                if(QRegExp("\\d*o").exactMatch(fileInfo.suffix()))
                {
                    QByteArray inputFilePath = fileInfo.absoluteFilePath().toLocal8Bit();
                    memset(rinexConvertConfig.RawFile,0,sizeof(rinexConvertConfig.RawFile));
                    memcpy(rinexConvertConfig.RawFile,inputFilePath.data(),inputFilePath.size());

                    auto newObject = QCObjectManager::getInstance()->addQCObject();
                    newObject->getQCConfigurator()->setRinexConvertAPIConfig(rinexConvertConfig);
                }
            }
            processThread.setQCObjects(QCObjectManager::getInstance()->getAllQCObjects());
            processThread.setProcessMode(QCProcessThread::RinexConvertMode);
            processThread.start();
            loop.exec();
        }
        fprintf(stdout,"\nProcess finished");
    }
#endif
    return 0;
}

bool CommandLineProcessor::parseCommandLine(QCoreApplication &app, CommandLineConfigData &parseResult)
{
    QCommandLineParser parser;
    parser.addPositionalArgument("inputPath","input files dir path");
    QCommandLineOption helpOption = parser.addHelpOption();
    QCommandLineOption versionOption = parser.addVersionOption();
    QCommandLineOption qcModeOption("q","Execute as QC mode");
    parser.addOption(qcModeOption);
    QCommandLineOption rinexConvertModeOption("r","Execute as Rinex convert mode");
    parser.addOption(rinexConvertModeOption);
    QCommandLineOption outputPathOption("o","Output dir path","path");
    parser.addOption(outputPathOption);
    QCommandLineOption configFilePathOption("c","Config file path","path");
    parser.addOption(configFilePathOption);

    if(!parser.parse(app.arguments()))
    {
        fprintf(stderr,"ERROR: %s, use '-h' for help information.",parser.errorText().toLocal8Bit().data());
        return false;
    }
    if(parser.isSet(helpOption))
    {
        qInfo("%s",parser.helpText().toLocal8Bit().data());
        return false;
    }
    if(parser.isSet(versionOption))
    {
        parser.showVersion();
    }
    if(!parser.isSet(qcModeOption) && !parser.isSet(rinexConvertModeOption))
    {
        fprintf(stderr,"ERROR: Please specify execution mode '-p' or '-r', use '-h' for help information.");
        return false;
    }
    if(parser.isSet(qcModeOption) && parser.isSet(rinexConvertModeOption))
    {
        fprintf(stderr,"ERROR: Please only specify one execution mode '-p' or '-r', use '-h' for help information.");
        return false;
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
        fprintf(stderr,"ERROR: Please specify input files dir path, use '-h' for help information.");
        return false;
    }
    parseResult.inputDirPath = parser.positionalArguments().at(0);
    QFileInfo inputPathInfo(parseResult.inputDirPath);
    if(!inputPathInfo.exists() || !inputPathInfo.isDir())
    {
        fprintf(stderr,"ERROR: Please specify correct input files dir path, use '-h' for help information.");
        return false;
    }
    if(parser.isSet(outputPathOption))
    {
        QFileInfo outDirInfo(parser.value(outputPathOption));
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
            fprintf(stderr,"ERROR: Please specify correct config files path, use '-h' for help information.");
            return false;
        }
        parseResult.configFilePath = configFilePathInfo.absoluteFilePath();
    }
    return true;
}

void CommandLineProcessor::onProgressRateChanged(int totalRate, QString currentQCName, int currentQCRate)
{
    consoleProgressBar(currentQCName,currentQCRate,totalRate);
}

void CommandLineProcessor::onProcessError(QString currentQCName, QString errorMsg)
{
    fprintf(stderr,"\nERROR: [%s] %s",currentQCName.toLocal8Bit().data(),errorMsg.toLocal8Bit().data());
}
