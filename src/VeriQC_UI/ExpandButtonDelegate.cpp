#include "ExpandButtonDelegate.h"
#include <QApplication>
#include <QPainter>
#include "QCObjectTableModel.h"

ExpandButtonDelegate::ExpandButtonDelegate(QObject *parent)
    :QStyledItemDelegate(parent)
{

}

void ExpandButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == TABLE_MODEL_COLUMN_INDEX_SYS && !isChildRow(index))
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        opt.textElideMode = Qt::ElideNone;

        QRect buttonRect(opt.rect.right() - 20, opt.rect.top() + (opt.rect.height()-16)/2, 16, 16);
        QPixmap icon;

        opt.text = index.data(Qt::DisplayRole).toString();
        if(opt.text.contains("(+)"))
        {
            opt.text = opt.text.split(" ")[0];
            icon = QPixmap(":/icons/Resources/Icons/Expand.png");
        }
        else if(opt.text.contains("(-)"))
        {
            opt.text = opt.text.split(" ")[0];
            icon = QPixmap(":/icons/Resources/Icons/Collapse.png");
        }
        painter->save();
        const QWidget *widget = opt.widget;
        QStyle *style = widget ? widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

        painter->drawPixmap(buttonRect, icon);
        painter->restore();
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

bool ExpandButtonDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonPress && index.column() == TABLE_MODEL_COLUMN_INDEX_SYS)
    {
        if (!isChildRow(index))
        {
            model->setData(index, QVariant(), Qt::EditRole);
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

bool ExpandButtonDelegate::isChildRow(const QModelIndex &index) const
{
     const QCObjectTableModel *model = qobject_cast<const QCObjectTableModel*>(index.model());
     if(model)
     {
         return !model->isParentRow(index.row());
     }
     return false;
}
