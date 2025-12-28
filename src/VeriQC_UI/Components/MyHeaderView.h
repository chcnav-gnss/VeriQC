/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Component Module
 * *-
 * @file MyHeaderView.h
 * @author CHC
 * @date 2025-09-28
 * @brief header view component of table view
 * 
**************************************************************************/
#ifndef MYHEADERVIEW_H
#define MYHEADERVIEW_H

#include <QHeaderView>

class MyHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    MyHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);
    void setCheckable(bool checkable);
    void setCheckState(Qt::CheckState state);
signals:
    void checkAll(bool checked);
protected:
    virtual void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
    virtual void mousePressEvent(QMouseEvent *event) override;
private:
    Qt::CheckState m_checkState = Qt::Unchecked;
    bool m_checkable = true;
};

#endif // MYHEADERVIEW_H
