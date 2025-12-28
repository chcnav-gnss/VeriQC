#ifndef FramelessWindow_H
#define FramelessWindow_H
#include "qsystemdetection.h"
#include <QObject>
#include <QMainWindow>  
#include<QVBoxLayout>
#include<QLabel>
#include<QPushButton>
#include<QToolButton>
#include<QStatusBar>
#include<QLineEdit>
#ifdef Q_OS_WIN
#include <QWidget>
#include <QList>
#include <QMargins>
#include <QRect>

class  FramelessWindow : public QWidget
{
	Q_OBJECT
public:
	explicit FramelessWindow(bool isNeedStatus, QWidget* parent = 0);
public:

	//if resizeable is set to false, then the window can not be resized by mouse
	//but still can be resized programtically
	void setResizeable(bool resizeable = true);
	bool isResizeable() { return m_bResizeable; }

	//set border width, inside this aera, window can be resized by mouse
	void setResizeableAreaWidth(int width = 5);
	void setCentralWidget(QWidget* widget);
	void setTitleBarCentralWidget(QWidget* widget);

	void showStatusMessage(QString msg);
	void setWindowCustomIcon(const QIcon& icon);
	void setWindowCustomTitle(const QString& text);
	void setWindowType(Qt::WindowType type);
protected:
	void addIgnoreWidget(QWidget* widget);
	void setTitleBarBackgroundColor(QColor m_color);
	virtual bool nativeEvent(const QByteArray& eventType, void* message, long* result); 
private:
	QList<QWidget*> m_whiteList;
	int m_borderWidth;
	bool isNeedStatusWidget;
	QMargins m_margins;
	QMargins m_frames;
	bool m_bResizeable;
	bool m_isMax;
	QVBoxLayout* m_verticalLayout;
	QLineEdit* m_statusTextEdit;
	QWidget* m_titleBar;
	QHBoxLayout* titleBarLayout;
	QPushButton* m_maximizeButton = nullptr;
	QPushButton* m_minimizeButton = nullptr;
	QPushButton* m_closeButton = nullptr;
	QWidget* m_titleCentralWidget;
	QVBoxLayout* m_titleCentralLayout;
	QWidget* centralWidget;
	QVBoxLayout* centralLayout;
	QWidget* m_statusWidget;
	QVBoxLayout* m_statusLayout;
	QLabel* m_imageLabel;
	QLabel* m_titleLabel;
	void initUI();
	void maximizeBtnClicked();
	void minimizeBtnClicked();
	void closeBtnClicked();
	QString getStyleByFileName(QString styleName);
signals:
	void closeWindowSign();
};
#endif
#endif // FramelessWindow_H
