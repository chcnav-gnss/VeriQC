#ifndef UPDATEMAINWINDOW_H_
#define UPDATEMAINWINDOW_H_ 
#include <QtWidgets/QMainWindow> 
#include<QVBoxLayout>
#include<QLabel>
#include<QThread>
#include<QProgressBar>
#include<QPushButton>
#include<QLineEdit>
#include<QComboBox>
#include<QWidget>
#include<QTranslator>
#include"FramelessWindow.h"
class UpgradeMainWindow : public QMainWindow
{
	Q_OBJECT
public:
    UpgradeMainWindow(QWidget* parent = nullptr);
    ~UpgradeMainWindow();
    void setData(QString currentVersion, QString serverVersion, QString url, int fileSize);
private:
	QVBoxLayout* m_vRootLayout;
    QLabel* m_descLabel;
    QLabel* m_updageProgressLabel;
    int m_downProgress = 0;
	int m_fileSize = 0;
	bool m_isClosed = false;
    QProgressBar* m_progressBar;
    QString m_downLoadPath;
    QThread* m_workerThread;
	QThread* m_checkProcessThread;
	QTranslator* m_trans;
	void setCurrentLanguage();
    bool checkProcessByQProcess(const QString& strExe);
	void checkExistProcess();
	void killSoftWare();
    void onZipExtractFinished(bool bSuccess);
private slots:
	void unzipFile();
Q_SIGNALS:
	void processClosedSignal();
};
#endif //UPDATEMAINWINDOW_H_
