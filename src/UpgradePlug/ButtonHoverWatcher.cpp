#include "ButtonHoverWatcher.h"
ButtonHoverWatcher::ButtonHoverWatcher(QString normalPath, QString hoverPath, QObject* parent) :
	QObject(parent)
{
	m_normalIconPath = normalPath;
	m_hoverIconPath = hoverPath;
}

bool ButtonHoverWatcher::eventFilter(QObject* watched, QEvent* event)
{
	QPushButton* button = qobject_cast<QPushButton*>(watched);
	if (!button) {
		return false;
	}
	if (event->type() == QEvent::Enter&& button->isEnabled())
	{ 
		button->setIcon(QIcon(m_hoverIconPath));
		return true;
	}

	if (event->type() == QEvent::Leave) {
		button->setIcon(QIcon(m_normalIconPath));
		return true;
	} 
	return false;
}