/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Module
 * *-
 * @file MainWindow.h
 * @author CHC
 * @date 2025-09-28
 * @brief main window
 * 
**************************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLocale>
#include <QTranslator>
#include "QCObjectTableModel.h"
#include "StationAttributeInfoWidget.h"
#include "Components/MyAlertMessageBox.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
signals:
    void dropedImportFiles(QStringList files);
protected:
    void onTableMenuRequested(const QPoint &pos);
    void onTableViewCurrentRowChanged();
    void onImportButtonClicked();
    void onRunQC();
    void onExportSelectedQCResults();
    void onExportSelectedQCReports();
    void onRinexConvertSetting();
    void onRinexConvert();
    void onGlobalQCSetting();
    void onAbout();
    void onShowUserManual();
    void onDrawObsTimePlot();
    void onDrawSkyPlot();
    void onDrawSNRAndMPTimePlot();
    void onDrawSatNumAndDOPTimePlot();
    void onMergeRinexFiles();
    void onTableDataChanged();
    void onQCObjectInputFormatChanged(int qcObjectId);

    void onTranslateChinese();
    void onTranslateEnglish();
    void setLanguage(QLocale::Language language);
    void retranslateUi();
    void setPlotable(bool plotable);

    QSharedPointer<QCObject> getCurrentQCObject();

    void importFiles(QStringList filePaths);
    void exportQCResults(const QVector<int> &qcObjects);

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    bool eventFilter(QObject *watched, QEvent *event) override;
    void resetWindow();
private:
    Ui::MainWindow *ui;
    QCObjectTableModel *m_tableModel = nullptr;
    QTranslator* m_pMainTranslator = nullptr;
    QTranslator* m_pQtTranslator = nullptr;
    StationAttributeInfoWidget *m_stationAttributeInfoWidget;
    MyAlertMessageBox *m_alertMessageBox;
};
#endif // MAINWINDOW_H
