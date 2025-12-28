#include "AboutDialog.h"
#include "ui_AboutDialog.h"
#include <QDesktopServices>
#include <QUrl>
#include "upgrade/UpgradeHelp.h"

AboutDialog::AboutDialog(QLocale::Language language, QWidget *parent) :
    MyDialog(parent),
    ui(new Ui::AboutDialog),
    m_language(language)
{
    ui->setupUi(this);

    connect(ui->checkUpdateButton,&QPushButton::clicked,this,&AboutDialog::onCheckUpdate);
    connect(ui->learnMoreLabel,&QLabel::linkActivated,this,&AboutDialog::onLearnMore);
    connect(ui->historicalVersionLabel,&QLabel::linkActivated,this,&AboutDialog::onHistoricalVersion);

    ui->versionLabel->setText(QCoreApplication::applicationVersion());
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::onCheckUpdate()
{
    UpgradeHelp(this,true).upgradeSoftWare();
}

void AboutDialog::onLearnMore()
{
    QDesktopServices::openUrl(
        QUrl("https://newwww.huace.cn/home.html", QUrl::TolerantMode));
}

void AboutDialog::onHistoricalVersion()
{
    QUrl historicalVersionUrl;
    if(QLocale::Language::Chinese == m_language)
    {
        historicalVersionUrl = QUrl::fromLocalFile(QCoreApplication::applicationDirPath().append("/releaseNote.txt"));
    }
    else
    {
        historicalVersionUrl = QUrl::fromLocalFile(QCoreApplication::applicationDirPath().append("/releaseNote_en.txt"));
    }

    QDesktopServices::openUrl(historicalVersionUrl);
}
