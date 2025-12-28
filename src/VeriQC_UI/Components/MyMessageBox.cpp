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
    messageBox.setStyleSheet("QLabel {min-width: 200px;}");
    messageBox.setWindowTitle(title);
    messageBox.setWindowIcon(icon);
    messageBox.setText(text);
    messageBox.setStandardButtons(buttons);
    messageBox.setDefaultButton(defaultButton);
    if (messageBox.exec() == -1)
        return QMessageBox::Cancel;
    return messageBox.standardButton(messageBox.clickedButton());
}
