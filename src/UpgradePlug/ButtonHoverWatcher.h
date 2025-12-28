#ifndef BUTTONHOVERWATCHER_H
#define BUTTONHOVERWATCHER_H
#include <QObject>
#include <QPushButton>
#include <QEvent> 

class  ButtonHoverWatcher : public QObject
{
	Q_OBJECT
public:
	explicit ButtonHoverWatcher(QString normalPath, QString hoverPath,QObject* parent = Q_NULLPTR);
	virtual bool eventFilter(QObject* watched, QEvent* event) Q_DECL_OVERRIDE;
private:
	QString m_normalIconPath;
	QString m_hoverIconPath;
};
#endif //BUTTONHOVERWATCHER_H
