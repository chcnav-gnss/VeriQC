/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Component Module
 * *-
 * @file MyMenu.h
 * @author CHC
 * @date 2025-09-28
 * @brief menu component
 * 
**************************************************************************/
#ifndef MYMENU_H
#define MYMENU_H

#include <QMenu>

class MyMenu : public QMenu
{
    Q_OBJECT
public:
    explicit MyMenu(QWidget *parent = nullptr);
    explicit MyMenu(const QString &title, QWidget *parent = nullptr);
protected:
    void init();
};

#endif // MYMENU_H
