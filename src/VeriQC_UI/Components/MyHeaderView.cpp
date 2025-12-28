#include "MyHeaderView.h"
#include <QStandardItemModel>
#include <QPainter>
#include <QMouseEvent>
#include <QCheckBox>

MyHeaderView::MyHeaderView(Qt::Orientation orientation, QWidget *parent)
    :QHeaderView(orientation,parent)
{
    setTextElideMode(Qt::ElideRight);
}

void MyHeaderView::setCheckable(bool checkable)
{
    m_checkable = checkable;
    this->update();
}

void MyHeaderView::setCheckState(Qt::CheckState state)
{
    m_checkState = state;
    update();
}

void MyHeaderView::mousePressEvent(QMouseEvent *event)
{
    if (m_checkable)
    {
        int column = logicalIndexAt(event->pos());
        if ((event->buttons() & Qt::LeftButton) && (column == 0))
        {
            bool isChecked = m_checkState == Qt::Checked;
            isChecked = !isChecked;
            emit checkAll(isChecked);
            m_checkState = isChecked ? Qt::Checked : Qt::Unchecked;
            updateSection(0);
        }
    }

    QHeaderView::mousePressEvent(event);
}

void MyHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();

    if(m_checkable && (logicalIndex == 0))
    {
        QStyleOptionButton option;
        option.initFrom(this);
        option.iconSize = QSize(12,12);
        option.rect = rect.adjusted(2,0,0,0);
        option.state = QStyle::State_Enabled;
        if(m_checkState == Qt::PartiallyChecked)
        {
            option.state |= QStyle::State_NoChange;
        }
        else
        {
            option.state |= (m_checkState == Qt::Checked) ?
                QStyle::State_On : QStyle::State_Off;
        }
        style()->drawControl(QStyle::CE_CheckBox,&option,painter,this->parentWidget());
    }
}
