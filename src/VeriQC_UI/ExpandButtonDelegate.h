/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Module
 * *-
 * @file ExpandButtonDelegate.h
 * @author CHC
 * @date 2025-09-28
 * @brief delegate for button in item of table view 
 * 
**************************************************************************/
#ifndef EXPANDBUTTONDELEGATE_H
#define EXPANDBUTTONDELEGATE_H

#include <QStyledItemDelegate>

class ExpandButtonDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ExpandButtonDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
protected:
    bool isChildRow(const QModelIndex &index)const;
};

#endif // EXPANDBUTTONDELEGATE_H
