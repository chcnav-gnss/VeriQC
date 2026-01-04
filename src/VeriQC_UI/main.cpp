#include "MainWindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QCObject.h>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QFontDatabase>
#include "AppSettingsManagement.h"
#include "CommandLineProcessor.h"

#define AUTO_UPGRADE_FILE_NAME  "AutoUpgradePlug.exe"

/**
  @brief  Move upgraded AutoUpdatePlug.exe to appication dir.
 */
static void copyUpgradeProc()
{
    QString path = QCoreApplication::applicationDirPath();
    QString upgradePath = QString("%1/%2/%3").arg(path).arg("UpgradeFile").arg(AUTO_UPGRADE_FILE_NAME);
    QString existPath = QString("%1/%2").arg(path).arg(AUTO_UPGRADE_FILE_NAME);
    QFile upgradeFile(upgradePath);
    QFile existFile(existPath);
    if (upgradeFile.exists())
    {
        if (existFile.exists())
        {
            existFile.remove();
        }
        bool isSuccess = upgradeFile.copy(existPath);
        if (isSuccess)
        {
            upgradeFile.remove();
        }
    }
}

void loadAppFont()
{
    int id = QFontDatabase::addApplicationFont(QApplication::applicationDirPath().append("/AlibabaPuHuiTi-3-55-Regular.ttf"));
    if (-1 != id)
    {
        QStringList families = QFontDatabase::applicationFontFamilies(id);
        if (families.size() > 0)
        {
            QString family = families.at(0);
            qApp->setFont(QFont(family,10));
        }
    }
}

//static QString getBuildDate()
//{
//    QString buildDateStr = __DATE__;
//    buildDateStr.replace("  ", " 0");
//    QDate buildDate = QLocale(QLocale::English).toDate(buildDateStr, "MMM dd yyyy");
//    return buildDate.toString("yyyyMMdd");
//}

int main(int argc, char *argv[])
{
#ifdef CONSOLE
        QCoreApplication a(argc, argv);
#ifdef APP_VERSION
        a.setApplicationVersion(APP_VERSION);
#endif // APP_VERSION
        SetExePath(a.applicationFilePath().toLocal8Bit().data());
        CommandLineProcessor processor;
        return processor.exec(a);
#else
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    //QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication a(argc, argv);
#ifdef APP_VERSION
    a.setApplicationVersion(APP_VERSION);
#endif // APP_VERSION
    SetExePath(a.applicationFilePath().toLocal8Bit().data());
    loadAppFont();
    copyUpgradeProc();

    QFile styleFile(":/qss/Resources/qss/style.qss");
    styleFile.open(QFile::ReadOnly);
    qApp->setStyleSheet(styleFile.readAll());
    styleFile.close();

    MainWindow w;
    w.show();

    int execode = a.exec();
    // upgrade
    QStringList upgradeArgs = AppSettingsManagement::getInstance()->getUpgradePlugArgs();
    if (upgradeArgs.count() > 0)
    {
        QString filePath = QCoreApplication::applicationDirPath() +
            QDir::separator() + AUTO_UPGRADE_FILE_NAME;
        QFileInfo fi(filePath);
        if (fi.exists())
        {
            QProcess* pt = new QProcess;
            pt->startDetached(filePath, upgradeArgs);
        }
    }
    return execode;
#endif
}
