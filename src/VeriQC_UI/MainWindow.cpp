#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QStyledItemDelegate>
#include "Components/MyMessageBox.h"
#include <QLibraryInfo>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include "ExpandButtonDelegate.h"
#include "QCObjectManager.h"
#include "QCProgressDialog.h"
#include "ImportFileDialog.h"
#include "QCConfigDialog.h"
#include "QCResultExporter.h"
#include "RinexConvertSettingDialog.h"
#include "AppSettingsManagement.h"
#include "AboutDialog.h"
#include "upgrade/UpgradeHelp.h"
#include "ObsTimePlotDialog.h"
#include "SkyPlotDialog.h"
#include "SNRAndMPTimePlotDialog.h"
#include "SatNumAndDOPTimePlotDialog.h"
#include "RinexMergeDialog.h"
#include "Components/MyHeaderView.h"
#include "Components/MyMenu.h"
#include "Components/MyAlertMessageBox.h"

class TableHighlightDelegate : public QStyledItemDelegate
{
public:
    explicit TableHighlightDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);

        QVariant fgColor = index.data(Qt::ForegroundRole);
        if (fgColor.isValid() && fgColor.canConvert<QColor>())
        {
            QColor textColor = fgColor.value<QColor>();

            if (option->state & QStyle::State_Selected)
            {
                option->palette.setColor(QPalette::HighlightedText, textColor);
            }
        }
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setAcceptDrops(true);
    ui->menubar->setFont(qApp->font());
    // init table view
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tableModel = new QCObjectTableModel(this);
    ui->tableView->setModel(m_tableModel);
    MyHeaderView *tableHeaderView = new MyHeaderView(Qt::Horizontal,ui->tableView);
    ui->tableView->setHorizontalHeader(tableHeaderView);
    connect(m_tableModel,&QCObjectTableModel::updateHeaderCheckState,tableHeaderView,&MyHeaderView::setCheckState);
    connect(m_tableModel,&QCObjectTableModel::dataChanged,this,&MainWindow::onTableDataChanged);
    connect(tableHeaderView,&MyHeaderView::checkAll,m_tableModel,&QCObjectTableModel::updateAllCheckedStates);

    TableHighlightDelegate *highlightDelegate = new TableHighlightDelegate(this);
    ui->tableView->setItemDelegate(highlightDelegate);
    ExpandButtonDelegate *expandButtonDelegate = new ExpandButtonDelegate(this);
    ui->tableView->setItemDelegateForColumn(TABLE_MODEL_COLUMN_INDEX_SYS,expandButtonDelegate);
    ui->tableView->setItemDelegateForColumn(TABLE_MODEL_COLUMN_INDEX_CHECK_BOX,new QStyledItemDelegate(this));
    connect(ui->tableView,&QTableView::customContextMenuRequested,this,&MainWindow::onTableMenuRequested);
    connect(ui->tableView->selectionModel(),&QItemSelectionModel::currentRowChanged,this,&MainWindow::onTableViewCurrentRowChanged);

    connect(ui->importButton,&QPushButton::clicked,this,&MainWindow::onImportButtonClicked);
    connect(ui->actionImport,&QAction::triggered,this,&MainWindow::onImportButtonClicked);
    connect(ui->actionQC,&QAction::triggered,this,&MainWindow::onRunQC);
    connect(ui->actionExportQCResult,&QAction::triggered,this,&MainWindow::onExportSelectedQCResults);
    connect(ui->actionExportQCReport,&QAction::triggered,this,&MainWindow::onExportSelectedQCReports);
    connect(ui->actionRinexConvertSetting,&QAction::triggered,this,&MainWindow::onRinexConvertSetting);
    connect(ui->actionRinexConvert,&QAction::triggered,this,&MainWindow::onRinexConvert);
    connect(ui->actionQCSetting,&QAction::triggered,this,&MainWindow::onGlobalQCSetting);
    connect(ui->actionAbout,&QAction::triggered,this,&MainWindow::onAbout);
    connect(ui->actionUserManual, &QAction::triggered, this, &MainWindow::onShowUserManual);
    connect(ui->actionObsTimePlot,&QAction::triggered,this,&MainWindow::onDrawObsTimePlot);
    connect(ui->actionSkyPlot,&QAction::triggered,this,&MainWindow::onDrawSkyPlot);
    connect(ui->actionSNRAndMPPlot,&QAction::triggered,this,&MainWindow::onDrawSNRAndMPTimePlot);
    connect(ui->actionSatNumAndDOPPlot,&QAction::triggered,this,&MainWindow::onDrawSatNumAndDOPTimePlot);
    connect(ui->actionFileMerge,&QAction::triggered,this,&MainWindow::onMergeRinexFiles);

    connect(ui->actionChinese,&QAction::triggered,this,&MainWindow::onTranslateChinese);
    connect(ui->actionEnglish,&QAction::triggered,this,&MainWindow::onTranslateEnglish);

    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(TABLE_MODEL_COLUMN_INDEX_NAME,QHeaderView::Interactive);
    ui->tableView->setColumnWidth(TABLE_MODEL_COLUMN_INDEX_NAME,120);   // file name
    ui->tableView->horizontalHeader()->setSectionResizeMode(TABLE_MODEL_COLUMN_INDEX_SYS,QHeaderView::Interactive);
    ui->tableView->setColumnWidth(TABLE_MODEL_COLUMN_INDEX_SYS,80);   // file name
    for (int i = TABLE_MODEL_COLUMN_INDEX_MP1; i < m_tableModel->columnCount(); ++i)
    {
        ui->tableView->horizontalHeader()->setSectionResizeMode(i,QHeaderView::Interactive);
        ui->tableView->setColumnWidth(i,10);
    }

    connect(this,&MainWindow::dropedImportFiles,this,&MainWindow::importFiles,Qt::QueuedConnection);

    this->setPlotable(false);

    m_stationAttributeInfoWidget = new StationAttributeInfoWidget(this);
    connect(m_stationAttributeInfoWidget,&StationAttributeInfoWidget::inputFormatChanged,this,&MainWindow::onQCObjectInputFormatChanged);

    m_alertMessageBox = new MyAlertMessageBox(this);
    m_alertMessageBox->hide();

    this->setLanguage(AppSettingsManagement::getInstance()->getLanguage());

    qApp->installEventFilter(this);
    this->resetWindow();

    ui->toolBar->widgetForAction(ui->actionObsTimePlot)->installEventFilter(this);
    ui->toolBar->widgetForAction(ui->actionSkyPlot)->installEventFilter(this);
    ui->toolBar->widgetForAction(ui->actionSatNumAndDOPPlot)->installEventFilter(this);
    ui->toolBar->widgetForAction(ui->actionSNRAndMPPlot)->installEventFilter(this);

    ui->actionExportQCResult->setToolTip(tr("Batch export of quality check results to Excel"));
    ui->actionExportQCReport->setToolTip(tr("HTML quality check report for a single file"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onTableMenuRequested(const QPoint &pos)
{
    QModelIndex index = ui->tableView->indexAt(pos);
    if(!index.isValid())
    {
        return;
    }
    QSharedPointer<QCObject> qcObject = m_tableModel->getQCObject(index.row());
    Q_ASSERT(qcObject != nullptr);

    MyMenu menu;
    QAction *qcAction = menu.addAction(tr("QC"));
    QAction *qcConfigAction = menu.addAction(tr("QC Settings"));
    QAction *convertRinexAction = menu.addAction(tr("Rinex Convert"));
    QAction *stationAttributeInfoAction = menu.addAction(tr("Station Attribute Information"));
    QAction *exportQCResultAction = menu.addAction(tr("Export QC Result"));
    QAction *deleteAction = menu.addAction(tr("Delete"));
    menu.addSeparator();
    QAction* batchDeleteAction = menu.addAction(tr("Batch Delete"));

    QAction * selected = menu.exec(ui->tableView->viewport()->mapToGlobal(pos));
    if(selected == qcAction)
    {
        QCProgressDialog progressDialog;
        progressDialog.execProcess({qcObject,},QCProcessThread::QCMode);

        m_tableModel->updateQCObject(qcObject->getID());
        this->onTableViewCurrentRowChanged();   // check whether qc object of current row is plotable
    }
    else if(selected == qcConfigAction)
    {
        QCConfigDialog dialog(this);
        dialog.setConfig(qcObject->getQCConfigurator()->getQCConfig(),qcObject->getQCConfigurator()->getCheckConfig());
        if(QDialog::Accepted == dialog.exec())
        {
            qcObject->getQCConfigurator()->setQCConfig(dialog.getQCConfig());
            qcObject->getQCConfigurator()->setCheckConfig(dialog.getQCCheckConfig());
        }
    }
    else if(selected == convertRinexAction)
    {
        QCProgressDialog progressDialog;
        progressDialog.execProcess({qcObject,},QCProcessThread::RinexConvertMode);
    }
    else if(selected == stationAttributeInfoAction)
    {
        m_stationAttributeInfoWidget->updateInfo(qcObject);
        m_stationAttributeInfoWidget->show();
    }
    else if(selected == exportQCResultAction)
    {
        this->exportQCResults({qcObject->getID()});
    }
    else if(selected == deleteAction)
    {
        QString name = qcObject->getName();   // station file name
        auto ret = MyMessageBox::question(this,tr("Delete"),tr("Do you want to delete %1?").arg(name));
        if(QMessageBox::Yes == ret)
        {
            m_tableModel->removeQCObject(qcObject->getID());
            QCObjectManager::getInstance()->deleteQCObject(qcObject->getID());
            if(QCObjectManager::getInstance()->getAllQCObjectIds().isEmpty())
            {
                this->resetWindow();
            }
        }
    }
    else if (selected == batchDeleteAction)
    {
        auto ret = MyMessageBox::question(this, tr("Delete"), tr("Do you want to batch delete all checked OBS ?"));
        if (QMessageBox::Yes == ret)
        {
            for (auto id : m_tableModel->getSelectedQcObjects())
            {
                m_tableModel->removeQCObject(id);
                QCObjectManager::getInstance()->deleteQCObject(id);
            }
            if (QCObjectManager::getInstance()->getAllQCObjectIds().isEmpty())
            {
                this->resetWindow();
            }
        }
    }
}

void MainWindow::onTableViewCurrentRowChanged()
{
    QModelIndex current = ui->tableView->currentIndex();
    bool plotable = false;
    QSharedPointer<QCObject> qcObject = m_tableModel->getQCObject(current.row());
    if(qcObject && QCObject::QCStatusNotExecuted != qcObject->getQCStatus())
    {
        plotable = true;
    }
    this->setPlotable(plotable);
}

void MainWindow::onImportButtonClicked()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(this);
    if(filePaths.isEmpty())
    {
        return;
    }

    this->importFiles(filePaths);
}

void MainWindow::onRunQC()
{
    QVector<int> selectedObjs = m_tableModel->getSelectedQcObjects();
    if(0 == selectedObjs.size())
    {
        return;
    }
    QVector<QSharedPointer<QCObject>> qcObjects;
    for(int id : selectedObjs)
    {
        qcObjects.append(QCObjectManager::getInstance()->getQCObjectByID(id));
    }
    QCProgressDialog progressDialog;
    progressDialog.execProcess(qcObjects,QCProcessThread::QCMode);

    for(int id : selectedObjs)
    {
        m_tableModel->updateQCObject(id);
    }
    // check whether qc object of current row is plotable
    this->onTableViewCurrentRowChanged();
}

void MainWindow::onExportSelectedQCResults()
{
    QVector<int> selectedObjs = m_tableModel->getSelectedQcObjects();
    QVector<int> doneQCObjs;
    for(int id: selectedObjs)
    {
        auto obj = QCObjectManager::getInstance()->getQCObjectByID(id);
        if(obj)
        {
            if(QCObject::QCResultStatusEnum::QCStatusNotExecuted != obj->getQCStatus())
            {
                doneQCObjs.append(id);
            }
        }
    }
    if(doneQCObjs.isEmpty())
    {
        return;
    }
    this->exportQCResults(doneQCObjs);
}

void MainWindow::onExportSelectedQCReports()
{
    for(int id : m_tableModel->getSelectedQcObjects())
    {
        QSharedPointer<QCObject> qcObject = QCObjectManager::getInstance()->getQCObjectByID(id);
        if(!qcObject)
        {
            continue;
        }
        if(QCObject::QCResultStatusEnum::QCStatusNotExecuted == qcObject->getQCStatus())
        {
            continue;
        }
        QDesktopServices::openUrl(QUrl::fromLocalFile(qcObject->getQCConfigurator()->getReportHtmlFilePath()));
    }
}

void MainWindow::onRinexConvertSetting()
{
    RinexConvertSettingDialog settingDialog(this);
    settingDialog.setRinexConvertSettings(AppSettingsManagement::getInstance()->getRinexConvertSettings());
    if(QDialog::Accepted == settingDialog.exec())
    {
        AppSettingsManagement::getInstance()->saveRinexConvertSettings(settingDialog.getRinexConvertSettings());
    }
}

void MainWindow::onRinexConvert()
{
    QVector<int> selectedObjs = m_tableModel->getSelectedQcObjects();
    if(0 == selectedObjs.size())
    {
        return;
    }
    QVector<QSharedPointer<QCObject>> qcObjects;
    for(int id : selectedObjs)
    {
        auto obj = QCObjectManager::getInstance()->getQCObjectByID(id);
        if(obj)
        {
            QCConfigurator::RawFileDataFormatEnum fileFormat = obj->getQCConfigurator()->getRawFileInfo().rawFileDataFormat;
            if(QCConfigurator::RawFileDataFormatEnum::None != fileFormat
                    && QCConfigurator::RawFileDataFormatEnum::RINEX != fileFormat)
            {
                qcObjects.append(obj);
            }
        }
    }
    if(qcObjects.isEmpty())
    {
        return;
    }
    QCProgressDialog progressDialog;
    progressDialog.execProcess(qcObjects,QCProcessThread::RinexConvertMode);
}

void MainWindow::onGlobalQCSetting()
{
    QCConfigDialog configDialog(this);
    configDialog.setConfig(AppSettingsManagement::getInstance()->getDefaultQCConfig(),
                           AppSettingsManagement::getInstance()->getDefaultQCCheckConfig());
    if(QDialog::Accepted == configDialog.exec())
    {
        AppSettingsManagement::getInstance()->saveDefaultQCConfig(configDialog.getQCConfig(),configDialog.getQCCheckConfig());
        for(auto obj : QCObjectManager::getInstance()->getAllQCObjects())
        {
            obj->getQCConfigurator()->setQCConfig(AppSettingsManagement::getInstance()->getDefaultQCConfig());
            obj->getQCConfigurator()->setCheckConfig(AppSettingsManagement::getInstance()->getDefaultQCCheckConfig());
        }
    }
}

void MainWindow::onAbout()
{
    QLocale::Language language = ui->actionChinese->isChecked() ? QLocale::Language::Chinese : QLocale::Language::English;
    AboutDialog dialog(language,this);
    dialog.exec();
}

void MainWindow::onShowUserManual()
{
    QString userManualFile = QApplication::applicationDirPath() + "/VeriQCUserManual_zh.pdf";
    QDesktopServices::openUrl(QUrl::fromLocalFile(userManualFile));
}

void MainWindow::onDrawObsTimePlot()
{
    QSharedPointer<QCObject> qcObject = this->getCurrentQCObject();
    if(qcObject)
    {
        ObsTimePlotDialog dialog(qcObject->getID(),this);
        dialog.resize(800,600);
        dialog.exec();
    }
}

void MainWindow::onDrawSkyPlot()
{
    QSharedPointer<QCObject> qcObject = this->getCurrentQCObject();
    if(qcObject)
    {
        SkyPlotDialog dialog(qcObject->getID(),this);
        dialog.resize(800,600);
        dialog.exec();
    }
}

void MainWindow::onDrawSNRAndMPTimePlot()
{
    QSharedPointer<QCObject> qcObject = this->getCurrentQCObject();
    if(qcObject)
    {
        SNRAndMPTimePlotDialog dialog(qcObject->getID(),this);
        dialog.resize(800,600);
        dialog.exec();
    }
}

void MainWindow::onDrawSatNumAndDOPTimePlot()
{
    QSharedPointer<QCObject> qcObject = this->getCurrentQCObject();
    if(qcObject)
    {
        SatNumAndDOPTimePlotDialog dialog(qcObject->getID(),this);
        dialog.resize(800,600);
        dialog.exec();
    }
}

void MainWindow::onMergeRinexFiles()
{
    RinexMergeDialog dialog(this);
    dialog.exec();
}

void MainWindow::onTableDataChanged()
{
    bool existsNonRinexObj = false;
    bool existsDoneQCObj = false;
    QVector<int> selectedObjs = m_tableModel->getSelectedQcObjects();
    for(int id: selectedObjs)
    {
        auto obj = QCObjectManager::getInstance()->getQCObjectByID(id);
        if(obj)
        {
            QCConfigurator::RawFileDataFormatEnum fileFormat = obj->getQCConfigurator()->getRawFileInfo().rawFileDataFormat;
            if(QCConfigurator::RawFileDataFormatEnum::None != fileFormat
                    && QCConfigurator::RawFileDataFormatEnum::RINEX != fileFormat)
            {
                existsNonRinexObj = true;
            }
            if(QCObject::QCResultStatusEnum::QCStatusNotExecuted != obj->getQCStatus())
            {
                existsDoneQCObj = true;
            }
        }
    }
    ui->actionQC->setEnabled(!selectedObjs.isEmpty());
    ui->actionRinexConvert->setEnabled(existsNonRinexObj);
    ui->actionExportQCReport->setEnabled(existsDoneQCObj);
    ui->actionExportQCResult->setEnabled(existsDoneQCObj);
    this->onTableViewCurrentRowChanged();
}

void MainWindow::onQCObjectInputFormatChanged(int qcObjectId)
{
    m_tableModel->updateQCObject(qcObjectId);

}

void MainWindow::onTranslateChinese()
{
    this->setLanguage(QLocale::Chinese);
}

void MainWindow::onTranslateEnglish()
{
    this->setLanguage(QLocale::English);
}

void MainWindow::setLanguage(QLocale::Language language)
{
    if (!m_pMainTranslator)
    {
        m_pMainTranslator = new QTranslator(this);
    }
    if (!m_pQtTranslator)
    {
        m_pQtTranslator = new QTranslator(this);
    }
    qApp->removeTranslator(m_pMainTranslator);
    qApp->removeTranslator(m_pQtTranslator);

    if (QLocale::Language::Chinese == language)
    {
        m_pMainTranslator->load(":/translators/VeriQC_UI_zh_CN.qm");
        m_pQtTranslator->load(QLocale::Language::Chinese,
            "qt", "_",
            QLibraryInfo::location(QLibraryInfo::TranslationsPath));

        qApp->installTranslator(m_pMainTranslator);
        qApp->installTranslator(m_pQtTranslator);
    }

    this->retranslateUi();
    m_stationAttributeInfoWidget->retranslateUi();

    ui->actionChinese->setChecked(QLocale::Language::Chinese == language);
    ui->actionEnglish->setChecked(QLocale::Language::Chinese != language);

    AppSettingsManagement::getInstance()->saveLanguage(language);
}

void MainWindow::retranslateUi()
{
    ui->retranslateUi(this);
    m_tableModel->retranslateUi();
}

void MainWindow::setPlotable(bool plotable)
{
    ui->actionObsTimePlot->setEnabled(plotable);
    ui->actionSkyPlot->setEnabled(plotable);
    ui->actionSNRAndMPPlot->setEnabled(plotable);
    ui->actionSatNumAndDOPPlot->setEnabled(plotable);
}

QSharedPointer<QCObject> MainWindow::getCurrentQCObject()
{
    QModelIndex currentIndex = ui->tableView->currentIndex();
    if(currentIndex.isValid())
    {
        QSharedPointer<QCObject> qcObject = m_tableModel->getQCObject(currentIndex.row());
        if(qcObject)
        {
            return qcObject;
        }
    }
    return nullptr;
}

void MainWindow::importFiles(QStringList filePaths)
{
    ImportFileDialog importFileDialog(this);
    importFileDialog.importFiles(filePaths);
    if(QDialog::Accepted != importFileDialog.exec())
    {
        return;
    }
    const QVector<QCConfigurator::QCConfigRawFileInfo> & configs = importFileDialog.getImportFileConfigs();
    if(configs.isEmpty())
    {
        return;
    }
    QVector<int> qcObjectIds;
    for(const QCConfigurator::QCConfigRawFileInfo &config : configs)
    {
        auto newObject = QCObjectManager::getInstance()->addQCObject();
        newObject->getQCConfigurator()->setRawFileInfo(config);
        qcObjectIds.append(newObject->getID());
    }

    m_tableModel->addQCObjects(qcObjectIds);

    ui->stackedWidget->setCurrentIndex(1);

    /** auto run QC after import */
    QVector<QSharedPointer<QCObject>> qcObjects;
    for (int id : qcObjectIds)
    {
        qcObjects.append(QCObjectManager::getInstance()->getQCObjectByID(id));
    }
    QCProgressDialog progressDialog;
    progressDialog.execProcess(qcObjects, QCProcessThread::QCMode);

    for (int id : qcObjectIds)
    {
        m_tableModel->updateQCObject(id);
    }
    // check whether qc object of current row is plotable
    this->onTableViewCurrentRowChanged();
}

void MainWindow::exportQCResults(const QVector<int> &qcObjects)
{
    QString filePath = QFileDialog::getSaveFileName(this,QString(),QString(),"xlsx (*.xlsx)");
    if(filePath.isEmpty())
    {
        return;
    }
    if(!QCResultExporter::exportToExcel(filePath,qcObjects))
    {
        MyMessageBox::critical(this,tr("Error"),tr("Export failed     "));
    }
    else
    {
        MyMessageBox::information(this,tr("Success"),tr("Export success     "));
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls())
    {
        QStringList filePaths;
        for (const QUrl &url: mimeData->urls())
        {
            QString filePath = url.toLocalFile();
            if (!filePath.isEmpty())
            {
                filePaths.append(filePath);
            }
        }
        emit dropedImportFiles(filePaths);
    }
    event->acceptProposedAction();
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);

    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->callOnTimeout([this,timer](){
        UpgradeHelp(this,false).upgradeSoftWare();
        timer->deleteLater();
    });
    timer->start(1000);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    if(m_stationAttributeInfoWidget->isVisible())
    {
        m_stationAttributeInfoWidget->setGeometry(
                    event->size().width() - m_stationAttributeInfoWidget->width()
                    ,0
                    ,m_stationAttributeInfoWidget->width()
                    ,event->size().height());
    }
    // resizeTableSections MP1 ~ SNR8
    const int minColumnWidth = QFontMetrics(ui->tableView->font()).horizontalAdvance("SNR9999...");
    const int columnCount = TABLE_MODEL_COLUMN_INDEX_SNR8 - TABLE_MODEL_COLUMN_INDEX_MP1 + 1;
    int columnWidth = 0;
    int viewSize = ui->tableView->isVisible() ? ui->tableView->width() : (ui->ImportPage->width() - 2 * ui->MainPage->layout()->contentsMargins().left());
    for (int i = TABLE_MODEL_COLUMN_INDEX_CHECK_BOX; i < TABLE_MODEL_COLUMN_INDEX_MP1; ++i)
    {
        viewSize -= ui->tableView->columnWidth(i);
    }
    if(viewSize < (minColumnWidth * columnCount))
    {
        columnWidth = minColumnWidth;
    }
    else
    {
        columnWidth = viewSize/columnCount;
    }
    for (int i = TABLE_MODEL_COLUMN_INDEX_MP1; i <= TABLE_MODEL_COLUMN_INDEX_SNR8; ++i)
    {
        ui->tableView->setColumnWidth(i,columnWidth);
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress && m_stationAttributeInfoWidget->isVisible()) {
        QPoint mousePos = mapFromGlobal(QCursor::pos());
        if (!m_stationAttributeInfoWidget->geometry().contains(mousePos)) {
            m_stationAttributeInfoWidget->hideWidget();
        }
    }
    if(event->type() == QEvent::MouseButtonRelease)
    {
        QVector<QObject*> plotActionWidgets = {
            ui->toolBar->widgetForAction(ui->actionObsTimePlot),
            ui->toolBar->widgetForAction(ui->actionSkyPlot),
            ui->toolBar->widgetForAction(ui->actionSatNumAndDOPPlot),
            ui->toolBar->widgetForAction(ui->actionSNRAndMPPlot) };
        if (plotActionWidgets.contains(watched))
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            QAction* action = ui->toolBar->actionAt(ui->toolBar->mapFromGlobal(mouseEvent->globalPos()));
            QVector<QAction*> plotActions = { ui->actionObsTimePlot,ui->actionSkyPlot,ui->actionSatNumAndDOPPlot,ui->actionSNRAndMPPlot };
            if (action && !action->isEnabled() && plotActions.contains(action))
            {
                m_alertMessageBox->setText(tr("Please click [QC] first, and this function will be activated after the QC is completed"));
                m_alertMessageBox->show();
                m_alertMessageBox->raise();
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::resetWindow()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->actionQC->setEnabled(false);
    ui->actionRinexConvert->setEnabled(false);
    ui->actionExportQCReport->setEnabled(false);
    ui->actionExportQCResult->setEnabled(false);
}
