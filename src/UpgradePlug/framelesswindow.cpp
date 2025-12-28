#include "FramelessWindow.h"
#include <QApplication>
#include <QPoint>
#include <QSize>
#ifdef Q_OS_WIN
#include <windows.h>
#include <WinUser.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <objidl.h> 
#include <gdiplus.h>
#include <GdiPlusColor.h>
#include<QWindow>
#include<QScreen> 
#include "ButtonHoverWatcher.h" 
#pragma comment (lib,"Dwmapi.lib")
#pragma comment (lib,"user32.lib")
FramelessWindow::FramelessWindow(bool isNeedStatus, QWidget* parent)
	: QWidget(parent),
	m_borderWidth(5),
	isNeedStatusWidget(isNeedStatus),
	m_bResizeable(true)
{
	setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
	setResizeable(m_bResizeable);
	initUI();
	
	setWindowCustomIcon(QIcon(":/CHCStarHubUpdatePlug/logo.ico"));
	QWindow* window = windowHandle();
	connect(window, &QWindow::screenChanged, this, [=]
		{
			auto hWnd = reinterpret_cast<HWND>(window->winId());
			SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		});
}
void FramelessWindow::initUI()
{
	setWindowIcon(QIcon(":/CHCStarHubUpdatePlug/logo.ico"));
	m_verticalLayout = new QVBoxLayout(this);
	m_verticalLayout->setSpacing(0);
	m_verticalLayout->setMargin(0);

	m_titleBar = new QWidget(this);
	setTitleBarBackgroundColor("#E4E4E4");
	m_titleBar->setFixedHeight(35);
	titleBarLayout = new QHBoxLayout(m_titleBar);
	titleBarLayout->setMargin(0);
	titleBarLayout->setContentsMargins(QMargins(3, 0, 0, 0));

	m_imageLabel = new QLabel(this);
	QSize fixSize(18, 18);
	m_imageLabel->setFixedSize(fixSize);
	titleBarLayout->addWidget(m_imageLabel);
	addIgnoreWidget(m_imageLabel);

	m_titleLabel = new QLabel(this);
	m_titleLabel->setStyleSheet("QLabel {color :#333333;font: normal 10pt Microsoft YaHei;}");
	m_titleLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	m_titleLabel->setFixedHeight(27);
	titleBarLayout->addWidget(m_titleLabel);
	addIgnoreWidget(m_titleLabel);
	m_titleLabel->setVisible(false);
	m_titleCentralWidget = new QWidget(this);
	m_titleCentralLayout = new QVBoxLayout(m_titleCentralWidget);
	m_titleCentralLayout->setMargin(0);
	m_titleCentralLayout->setSpacing(0);
	titleBarLayout->addWidget(m_titleCentralWidget);

	QWidget* rightSpacer = new QWidget(m_titleBar);
	rightSpacer->setFixedHeight(30);
	rightSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	titleBarLayout->addWidget(rightSpacer);
	addIgnoreWidget(rightSpacer);

 

	m_minimizeButton = new QPushButton(this);
	m_minimizeButton->setIconSize(QSize(10,10));
	m_minimizeButton->setStyleSheet(getStyleByFileName(":/CHCStarHubUpdatePlug/TitleBarMinButton.qss"));

	m_maximizeButton = new QPushButton(this);
	m_maximizeButton->setIconSize(QSize(10, 10));
	m_maximizeButton->setStyleSheet(getStyleByFileName(":/CHCStarHubUpdatePlug/TitleBarNormalButton.qss"));

	m_closeButton = new QPushButton(this);
	m_closeButton->setIconSize(QSize(10, 10));
	QString normalPath = ":/CHCStarHubUpdatePlug/Image/titlebar_closeNomal.png";
	QString hoverPath = ":/CHCStarHubUpdatePlug/Image/titlebar_closeHover.png";
	m_closeButton->setIcon(QIcon(normalPath));
	ButtonHoverWatcher* watcherClose = new ButtonHoverWatcher(normalPath, hoverPath, this);
	m_closeButton->installEventFilter(watcherClose);
	m_closeButton->setStyleSheet(getStyleByFileName(":/CHCStarHubUpdatePlug/TitleBarCloseButton.qss"));
	m_maximizeButton->setCheckable(true);
	m_isMax = true;
	 
	m_minimizeButton->setFixedSize(QSize(45,32));
	m_maximizeButton->setFixedSize(QSize(45, 32));
	m_closeButton->setFixedSize(QSize(45, 32));
	m_closeButton->setFocusPolicy(Qt::NoFocus);
	m_minimizeButton->setFocusPolicy(Qt::NoFocus);
	m_maximizeButton->setFocusPolicy(Qt::NoFocus);
	connect(m_minimizeButton, &QPushButton::clicked, this, &FramelessWindow::minimizeBtnClicked);
	connect(m_maximizeButton, &QPushButton::clicked, this, &FramelessWindow::maximizeBtnClicked);
	connect(m_closeButton, &QPushButton::clicked, this, &FramelessWindow::closeBtnClicked);

	titleBarLayout->addWidget(m_minimizeButton);
	titleBarLayout->addWidget(m_maximizeButton);
	titleBarLayout->addWidget(m_closeButton);
	titleBarLayout->setAlignment(m_minimizeButton, Qt::AlignTop);
	titleBarLayout->setAlignment(m_maximizeButton, Qt::AlignTop);
	titleBarLayout->setAlignment(m_closeButton, Qt::AlignTop);
	m_titleBar->layout();

	m_verticalLayout->addWidget(m_titleBar);
	centralWidget = new QWidget(this);
	centralLayout = new QVBoxLayout(centralWidget);
	centralWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	centralLayout->setSpacing(0);
	centralLayout->setMargin(0);
	m_verticalLayout->addWidget(centralWidget); 
}
QString FramelessWindow::getStyleByFileName(QString styleName)
{
	QString style;
	QFile buttonqss(styleName);
	if (buttonqss.open(QFile::ReadOnly))
	{
		style = QLatin1String(buttonqss.readAll());
		buttonqss.close();
		return  style;
	}
	return "";
}
void FramelessWindow::setTitleBarBackgroundColor(QColor m_color)
{
	QPalette pal(m_titleBar->palette());
	pal.setColor(QPalette::Background, m_color);
	m_titleBar->setAutoFillBackground(true);
	m_titleBar->setPalette(pal);
}

void FramelessWindow::setWindowCustomIcon(const QIcon& icon)
{
	QPixmap m_pic = icon.pixmap(icon.actualSize(QSize(18,18)));
	m_imageLabel->setPixmap(m_pic);

}
void  FramelessWindow::setWindowType(Qt::WindowType type)
{
	bool isShowMinButton = false;
	bool isShowMaxButton = false;
	bool isShowCloseButton = false;
	if (type == Qt::WindowMinimizeButtonHint)
	{
		isShowMinButton = true;
	}
	if (type == Qt::WindowMaximizeButtonHint)
	{
		isShowMaxButton = true;
	}
	if (type == Qt::WindowCloseButtonHint)
	{
		isShowCloseButton = true;
	}
	if (isShowMaxButton == false)
	{
		m_isMax = false;
		m_maximizeButton->setVisible(false);
	}
	if (isShowCloseButton == false)
	{
		m_closeButton->setVisible(false);
	}
	if (isShowMinButton == false)
	{
		m_minimizeButton->setVisible(false);
	}
}
void FramelessWindow::setWindowCustomTitle(const QString& text)
{
	m_titleLabel->setVisible(true);
	m_titleLabel->setText(text);
}
void  FramelessWindow::closeBtnClicked()
{
	close();
	emit closeWindowSign();
}
void FramelessWindow::maximizeBtnClicked()
{
	if (isMaximized())
	{
		QWidget::setContentsMargins(QMargins(0, 0, 0, 0));
		showNormal();
	}
	else
	{
		showMaximized();
	}
}
void FramelessWindow::showStatusMessage(QString msg)
{
	m_statusTextEdit->setText(msg);
}

void FramelessWindow::setTitleBarCentralWidget(QWidget* widget)
{
	m_titleCentralLayout->addWidget(widget);
}
void FramelessWindow::setCentralWidget(QWidget* widget)
{
	centralLayout->addWidget(widget);
}
void FramelessWindow::minimizeBtnClicked()
{
	if (isMinimized())
		setWindowState(windowState() & ~Qt::WindowMinimized);
	else
		setWindowState(windowState() | Qt::WindowMinimized);
}

void FramelessWindow::setResizeable(bool resizeable)
{
	bool visible = isVisible();
	m_bResizeable = resizeable;
	if (m_bResizeable) {
		setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
		HWND hwnd = (HWND)this->winId();
		DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
		::SetWindowLong(hwnd, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);
	}
	else {
		setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
		HWND hwnd = (HWND)this->winId();
		DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
		::SetWindowLong(hwnd, GWL_STYLE, style & ~WS_MAXIMIZEBOX & ~WS_CAPTION);
	}

	//we better left 1 piexl width of border untouch, so OS can draw nice shadow around it
	const MARGINS shadow = { 1, 1, 1, 1 };
	DwmExtendFrameIntoClientArea(HWND(winId()), &shadow);

	setVisible(visible);
}

void FramelessWindow::setResizeableAreaWidth(int width)
{
	if (1 > width) width = 1;
	m_borderWidth = width;
}

void FramelessWindow::addIgnoreWidget(QWidget* widget)
{
	if (!widget) return;
	if (m_whiteList.contains(widget)) return;
	m_whiteList.append(widget);
}

bool FramelessWindow::nativeEvent(const QByteArray& eventType, void* message, long* result)
{
#if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
	MSG* msg = *reinterpret_cast<MSG**>(message);
#else
	MSG* msg = reinterpret_cast<MSG*>(message);
#endif 
	switch (msg->message)
	{
	case WM_WINDOWPOSCHANGED:
	{
		return false;
	}
	case WM_NCCALCSIZE:
	{
		*result = 0;
		return true;
		break;
	}
	case WM_NCRBUTTONUP:
	{
		return false;
	}
	case WM_NCHITTEST:
	{
		*result = 0;
		const LONG border_width = m_borderWidth;
		RECT winrect;
		GetWindowRect(HWND(winId()), &winrect);
		long x = GET_X_LPARAM(msg->lParam);
		long y = GET_Y_LPARAM(msg->lParam);
		if (m_bResizeable)
		{ 
			bool resizeWidth = 1100 != maximumWidth();
			bool resizeHeight = 680 != maximumHeight();
			if (resizeWidth)
			{
				//left border
				if (x >= winrect.left && x < winrect.left + border_width)
				{
					*result = HTLEFT;
				}
				//right border
				if (x < winrect.right && x >= winrect.right - border_width)
				{
					*result = HTRIGHT;
				}
			}
			if (resizeHeight)
			{
				//bottom border
				if (y < winrect.bottom && y >= winrect.bottom - border_width)
				{
					*result = HTBOTTOM;
				}
				//top border
				if (y >= winrect.top && y < winrect.top + border_width)
				{
					*result = HTTOP;
				}
			}
			if (resizeWidth && resizeHeight)
			{
				//bottom left corner
				if (x >= winrect.left && x < winrect.left + border_width &&
					y < winrect.bottom && y >= winrect.bottom - border_width)
				{
					*result = HTBOTTOMLEFT;
				}
				//bottom right corner
				if (x < winrect.right && x >= winrect.right - border_width &&
					y < winrect.bottom && y >= winrect.bottom - border_width)
				{
					*result = HTBOTTOMRIGHT;
				}
				//top left corner
				if (x >= winrect.left && x < winrect.left + border_width &&
					y >= winrect.top && y < winrect.top + border_width)
				{
					*result = HTTOPLEFT;
				}
				//top right corner
				if (x < winrect.right && x >= winrect.right - border_width &&
					y >= winrect.top && y < winrect.top + border_width)
				{
					*result = HTTOPRIGHT;
				}
			}
		}

		if (0 != *result) return true;
		if (!m_titleBar) return false;
		double dpr = this->devicePixelRatioF();
		QPoint pos = m_titleBar->mapFromGlobal(QPoint(x / dpr, y / dpr));
		if (!m_titleBar->rect().contains(pos)) return false;
		QWidget* child = m_titleBar->childAt(pos);
		if (!child)
		{
			*result = HTCAPTION;
			return true;
		}
		else {
			if (m_whiteList.contains(child))
			{
				*result = HTCAPTION;
				return true;
			}
		}
		return false;
	} //end case WM_NCHITTEST
	case WM_GETMINMAXINFO:
	{
		if (::IsZoomed(msg->hwnd)) {
			RECT frame = { 0, 0, 0, 0 };
			AdjustWindowRectEx(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0);

			//record frame area data
			double dpr = this->devicePixelRatioF();

			m_frames.setLeft(abs(frame.left) / dpr + 0.5);
			m_frames.setTop(abs(frame.bottom) / dpr + 0.5);
			m_frames.setRight(abs(frame.right) / dpr + 0.5);
			m_frames.setBottom(abs(frame.bottom) / dpr + 0.5);
			QWidget::setContentsMargins(m_frames.left() + m_margins.left(), \
				m_frames.top() + m_margins.top(), \
				m_frames.right() + m_margins.right(), \
				m_frames.bottom() + m_margins.bottom());
			m_maximizeButton->setChecked(true);
			m_isMax = true;
		}
		else
		{
			if (m_isMax)
			{ 
				QWidget::setContentsMargins(m_margins);
				m_frames = QMargins();
				m_maximizeButton->setChecked(false);
				m_isMax = false;
				return true;
			}
		}
		return false;
	} 
	default:
		return QWidget::nativeEvent(eventType, message, result);
	}
}

#endif //Q_OS_WIN
