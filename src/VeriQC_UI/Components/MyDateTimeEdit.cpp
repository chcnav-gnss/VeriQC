#include "MyDateTimeEdit.h"
#include <QCalendarWidget>
#include <QStyleFactory>

MyDateTimeEdit::MyDateTimeEdit(QWidget *parent)
    :QDateTimeEdit(parent)
{
    this->setCalendarPopup(true);
    QCalendarWidget *calendarWidget = new QCalendarWidget(this);
    this->setCalendarWidget(calendarWidget);
    setDisplayFormat("yyyy/MM/dd HH:mm:ss");

}
