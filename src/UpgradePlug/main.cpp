#include "UpgradeMainWindow.h"
#include <QtWidgets/QApplication>
#include <QTimer>
#include <QFontDatabase>

int main(int argc, char* argv[])
{
	//QString currentVersion = "1.0.0.37";
	//QString serverVersion = "1.0.0.38";
	//QString url = "http://122.112.214.242:8090/fileStorage/download/03f09882a7713e00118bdd8702c2a9fa";
	//int totalSize = 7935369;
	QApplication a(argc, argv);
    QString currentVersion;
    QString serverVersion;
    QString url;
    int totalSize = 0;
	for (int i = 0; i < argc; i++)
	{
		if (i == 1)
		{
			currentVersion = argv[i];
		}
		else if (i == 2)
		{
			serverVersion = argv[i];
		}
		else if (i == 3)
		{
			url = argv[i];
		}
		else if (i == 4)
		{
			QString sizeStr = argv[i];
			bool ok;
			totalSize = sizeStr.toInt(&ok);
		}
	}

	UpgradeMainWindow w;
	/*w.SetData(currentVersion, serverVersion, url, totalSize);*/
	w.show();
	QTimer timer;
	timer.setSingleShot(true);
	QObject::connect(&timer, &QTimer::timeout, [&w, currentVersion, serverVersion, url, totalSize]() {
        w.setData(currentVersion, serverVersion, url, totalSize);
		});
	timer.start(200);
	return a.exec();
}
