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
    void onExportSelectedQCDiagnosis();
    void onRinexConvertSetting();
    void onRinexConvert();
    void onGlobalQCSetting();
    void onQCConfigFileEditor();    /**< Opens the GNSS QC config file editor. */
    void onRinexConfigFileEditor(); /**< Opens the RINEX convert config file editor. */
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
    void exportQCDiagnosis(const QVector<int> &qcObjects);

    /**< Run QC end-to-end on the given objects: progress dialog, exception
     *   dialog, per-object table refresh, current-row plotability update.
     *   Shared by the QC actions, post-import auto QC, and the config-change
     *   re-QC prompt. */
    void runQCAndRefresh(const QVector<QSharedPointer<QCObject>> &qcObjects);

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    bool eventFilter(QObject *watched, QEvent *event) override;
    void resetWindow();
private:
    /**< Collect a QC object's real observation start/end times (only valid
     *   after QC has run; returns a pair of invalid QDateTime otherwise).
     *   Used by the per-object and global config dialogs to populate the
     *   time-range edits' read-only reference value on unchecked rows. */
    static void getQCObjectObsTimeRange(const QSharedPointer<QCObject> &qcObject,
                                        QDateTime &obsStartTime,
                                        QDateTime &obsEndTime);
    /**< Decide whether an object's ephemeris settings diverge from the current
     *   global default and should therefore survive a global-config save.
     *   Divergence = different mode, or same custom-file mode with a different
     *   nav path (the path is irrelevant in the other modes). */
    static bool isEphemerisCustomized(const QCConfigurator::QCConfigData &objCfg,
                                      const QCConfigurator::QCConfigData &globalCfg);
    Ui::MainWindow *ui;
    QCObjectTableModel *m_tableModel = nullptr;
    QTranslator* m_pMainTranslator = nullptr;
    QTranslator* m_pQtTranslator = nullptr;
    StationAttributeInfoWidget *m_stationAttributeInfoWidget;
    MyAlertMessageBox *m_alertMessageBox;
};
#endif // MAINWINDOW_H
