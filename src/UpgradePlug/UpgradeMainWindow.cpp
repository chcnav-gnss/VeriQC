#include "UpgradeMainWindow.h"
#include "HttpClient.h"
#include<QDir>
#include<QProcess>
#include<QCoreApplication>
#include <qstandardpaths.h>
#include <qsettings.h>
#include<QMetaEnum>
#include "zip/CZipExtractor.h"
UpgradeMainWindow::UpgradeMainWindow(QWidget* parent)
	:QMainWindow(parent)
{
	setCurrentLanguage();
    setWindowTitle(tr("AutoUpgrade"));
	//setTitleBarBackgroundColor(QColor("#F5F5F5"));
	QPalette pal(this->palette());
	pal.setColor(QPalette::Background, QColor("#FFFFFF"));
	this->setAutoFillBackground(true);
	this->setPalette(pal);
	QWidget* mainWidget = new QWidget(this);
	setMinimumSize(500, 300);
	m_vRootLayout = new QVBoxLayout(mainWidget);
	m_vRootLayout->setAlignment(Qt::AlignCenter);
	m_vRootLayout->setMargin(20);
    m_descLabel = new QLabel(this);
    m_descLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_descLabel->setFixedHeight(30);
    m_vRootLayout->addWidget(m_descLabel);

	m_updageProgressLabel = new QLabel(this);
	m_updageProgressLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_updageProgressLabel->setFixedHeight(30);
	m_updageProgressLabel->setAlignment(Qt::AlignCenter);
	m_vRootLayout->addWidget(m_updageProgressLabel);

	m_vRootLayout->addSpacing(5);
	m_progressBar = new QProgressBar(this);
	m_progressBar->setStyleSheet("QProgressBar{background-color:#E7E7E7;border: 0px solid #DDDDDD;border-radius:5px;}"
		"QProgressBar::chunk{background-color:#1A92F3; border-radius: 5px;}");
	m_progressBar->setFormat(" %v%");
	m_progressBar->setMinimum(0);
	m_progressBar->setMaximum(100);
	m_progressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_progressBar->setAlignment(Qt::AlignCenter);
	m_progressBar->setFixedHeight(30);
	QString text = m_progressBar->format();
	text.replace("%v", QString::number(m_progressBar->value()));
	m_vRootLayout->addWidget(m_progressBar);
	m_vRootLayout->addSpacing(90);

	setCentralWidget(mainWidget);
    m_downLoadPath = QCoreApplication::applicationDirPath() +"/tmp.zip";

	if (QFile::exists(m_downLoadPath))
	{
		QFile file(m_downLoadPath);
		file.remove();
	}
}
void UpgradeMainWindow::setData(QString currentVersion, QString serverVersion, QString url, int fileSize)
{
	m_updageProgressLabel->setText(tr("downLoadFile..."));
	QString title = currentVersion + "->" + serverVersion;
    m_descLabel->setText(title);
	m_fileSize = fileSize;
	HttpClient(url).downloadProgress([this](const QByteArray& response)
		{
			m_downProgress += response.size();
			double precent = 0;
			if (m_progressBar->maximum() > 0)
				precent = ((double)m_downProgress / (double)m_fileSize) * 100;
			m_progressBar->setValue(precent);
		}).
		complete([this]()
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				m_updageProgressLabel->setText(tr("updateFile..."));
				m_progressBar->setValue(0);
				m_downProgress = 0;
				m_fileSize = 0;
                m_checkProcessThread = QThread::create(&UpgradeMainWindow::checkExistProcess, this);
                connect(this, &UpgradeMainWindow::processClosedSignal, this, &UpgradeMainWindow::unzipFile);
				m_checkProcessThread->start();
			}).download(m_downLoadPath);
}
bool UpgradeMainWindow::checkProcessByQProcess(const QString& strExe)
{
	bool bResult = false;
#ifdef _WIN32
	QProcess tasklist;
	tasklist.start("tasklist",
		QStringList() << "/NH"
		<< "/FO" << "CSV"
		<< "/FI" << QString("IMAGENAME eq %1").arg(strExe));
	tasklist.waitForFinished();
	QString strOutput = tasklist.readAllStandardOutput();
	if (strOutput.startsWith(QString("\"%1").arg(strExe)))
	{
		bResult = true;
	}
#else	/**< linux */
	QProcess tasklist;
	tasklist.start("/bin/bash", QStringList() << "-c" << QString("ps -ef | grep %1 | grep -v grep ").arg(strExe));
	tasklist.waitForFinished();
	QString strOutput = tasklist.readAllStandardOutput();
	if (strOutput.contains(QString("%1").arg(strExe)))
	{
		bResult = true;
	}
#endif // !LINUX
	return bResult;
}
void UpgradeMainWindow::checkExistProcess()
{
	while (!m_isClosed)
	{
        bool isExistMain = checkProcessByQProcess("VeriQC_UI.exe");
		if (isExistMain)
		{
#ifdef _WIN32
            QProcess::startDetached("taskkill -t -f /IM VeriQC_UI.exe");
#else	/**< linux */
            QProcess::startDetached("/bin/bash",QStringList()<<"-c" <<"ps -ef | grep VeriQC_UI.exe | grep -v grep | awk '{print $2}' | xargs kill -9");
#endif // !LINUX
		}
		if (isExistMain == false)
		{
			emit processClosedSignal();
			break;
		}
	}
}
void UpgradeMainWindow::unzipFile()
{ 
	if (QFile::exists(m_downLoadPath))
	{ 
		QString filePath = QCoreApplication::applicationDirPath();
		CZipExtractor* pZipExtractor = new CZipExtractor(this);
        connect(pZipExtractor, &CZipExtractor::ExtractFinished, this, &UpgradeMainWindow::onZipExtractFinished);
		connect(pZipExtractor, &CZipExtractor::ProgressUpdateSignal, m_progressBar, &QProgressBar::setValue);
		pZipExtractor->StartUnzip(m_downLoadPath, filePath);
	}
}

void UpgradeMainWindow::setCurrentLanguage()
{
	m_trans = new QTranslator(qApp);
	QLocale::Language eLangType = QLocale::system().language();

	QString tmp;
	if (eLangType == QLocale::Chinese)
	{
        tmp = ":/UpgradePlug/Translation_Upgrade_zh.qm";
	}
	m_trans->load(tmp);
	qApp->installTranslator(m_trans);
}

void UpgradeMainWindow::killSoftWare()
{
	QString filePath = QCoreApplication::applicationDirPath() +
        QDir::separator() + "VeriQC_UI.exe";
	QFileInfo fi(filePath);
	if (!fi.exists())
	{
		return;
	}
	QProcess* pt = new QProcess;
	pt->startDetached(filePath);
	qApp->exit(0);
}
void UpgradeMainWindow::onZipExtractFinished(bool bSuccess)
{
	if (bSuccess) {
		m_updageProgressLabel->setText(tr("extractcomplete"));
		m_progressBar->setValue(100);
		killSoftWare();
	}
	else {
		m_updageProgressLabel->setText(tr("extractfailure"));
	}
}
UpgradeMainWindow::~UpgradeMainWindow()
{
	m_isClosed = true;
	if (m_checkProcessThread)
	{
		m_checkProcessThread->quit();
		m_checkProcessThread->wait();
	}
	if (m_workerThread)
	{
		m_workerThread->quit();
		m_workerThread->wait();
	}
	if (m_trans)
	{
		delete m_trans;
	}
}
