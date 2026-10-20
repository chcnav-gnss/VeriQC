#include "MyMessageBox.h"
#include <QStyle>

MyMessageBox::MyMessageBox(QWidget *parent)
    :QDialog(parent)
{

}

QMessageBox::StandardButton MyMessageBox::information(QWidget *parent, const QString &title, const QString &text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
    QIcon icon = QDialog().style()->standardIcon(QStyle::SP_MessageBoxInformation);
    return execMessageBox(parent,icon,title,text,buttons,defaultButton);
}

QMessageBox::StandardButton MyMessageBox::question(QWidget *parent, const QString &title, const QString &text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
    QIcon icon = QDialog().style()->standardIcon(QStyle::SP_MessageBoxQuestion);
    return execMessageBox(parent,icon,title,text,buttons,defaultButton);
}

QMessageBox::StandardButton MyMessageBox::warning(QWidget *parent, const QString &title, const QString &text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
    QIcon icon = QDialog().style()->standardIcon(QStyle::SP_MessageBoxWarning);
    return execMessageBox(parent,icon,title,text,buttons,defaultButton);
}

QMessageBox::StandardButton MyMessageBox::critical(QWidget *parent, const QString &title, const QString &text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
    QIcon icon = QDialog().style()->standardIcon(QStyle::SP_MessageBoxCritical);
    return execMessageBox(parent,icon,title,text,buttons,defaultButton);
}

QMessageBox::StandardButton MyMessageBox::execMessageBox(QWidget *parent, QIcon icon, const QString &title, const QString &text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
    QMessageBox messageBox(parent);
    /**< Do NOT apply geometry stylesheets (e.g. "QLabel { min-width: ... }")
     *   to a QMessageBox: QMessageBoxPrivate::updateSize() measures the
     *   layout minimum width with word wrap forced off, and an explicit QSS
     *   min-width on the message label REPLACES (not maxes with) the label's
     *   minimumSizeHint. The box then gets fixed to roughly that min-width
     *   and any wider text is hard-clipped on a single line (verified
     *   against Qt 5.12.12: a 12.5em rule collapsed the box to ~268px and
     *   truncated long CJK prompts at ~40%). */
    messageBox.setWindowTitle(title);
    messageBox.setWindowIcon(icon);
    messageBox.setText(text);
    messageBox.setStandardButtons(buttons);
    messageBox.setDefaultButton(defaultButton);
    if (messageBox.exec() == -1)
        return QMessageBox::Cancel;
    return messageBox.standardButton(messageBox.clickedButton());
}
