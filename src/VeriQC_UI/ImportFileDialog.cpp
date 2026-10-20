#include "ImportFileDialog.h"
#include "ui_ImportFileDialog.h"
#include <QScreen>
#include <QGuiApplication>
#include <QFileInfo>
#include "Components/MyComboBox.h"
#include <QListView>
#include "Components/MyMessageBox.h"
#include "AppSettingsManagement.h"
#include "Components/MyHeaderView.h"


ImportFileDialog::ImportFileDialog(QWidget *parent) :
    MyDialog(parent),
    ui(new Ui::ImportFileDialog)
{
    ui->setupUi(this);

    QStringList tableHeaders = QStringList() <<"" << tr("Num") << tr("File Name") << tr("File Path") << tr("File Size") << tr("Format") << tr("RTCM Time");
    m_tableHeaderView = new MyHeaderView(Qt::Orientation::Horizontal, ui->tableWidget);
    m_tableHeaderView->setCheckable(false);
    connect(m_tableHeaderView, &MyHeaderView::checkAll, this, &ImportFileDialog::onCheckAll);
    ui->tableWidget->setColumnCount(tableHeaders.size());
    ui->tableWidget->setHorizontalHeaderLabels(tableHeaders);
    ui->tableWidget->setHorizontalHeader(m_tableHeaderView);
    connect(ui->tableWidget, &QTableWidget::itemClicked, this, &ImportFileDialog::onTableItemClicked);

    connect(ui->importButton,&QPushButton::clicked,this,&ImportFileDialog::onImport);
    connect(ui->cancelButton,&QPushButton::clicked,this,&ImportFileDialog::reject);
    connect(ui->batchApplyButton,&QPushButton::clicked,this,&ImportFileDialog::onApply);
    connect(ui->batchFormattingButton, &QPushButton::clicked, this, &ImportFileDialog::onBatchFormatting);
    connect(ui->cancelBatchFormattingButton, &QPushButton::clicked, this, &ImportFileDialog::onCancelBatchFormatting);

    QCConfigurator::addFileDataFormatItems(ui->inputFormatComboBox);
    connect(ui->inputFormatComboBox,&QComboBox::currentTextChanged,this,&ImportFileDialog::onInputFormatComboBoxChanged);

    ui->rtcmTimeEdit->setDateTime(QDateTime::currentDateTime());

    /** Column widths and minimum size use logical pixels.
     *  Qt AA_EnableHighDpiScaling handles DPI scaling automatically;
     *  multiplying by devicePixelRatio() causes double-scaling on HiDPI displays. */
    ui->tableWidget->setColumnWidth(TableWidgetColumnIndexEnum::ColumnIndexCheckBox, 0);
    ui->tableWidget->setColumnWidth(TableWidgetColumnIndexEnum::ColumnIndexNum, 50);
    ui->tableWidget->setColumnWidth(TableWidgetColumnIndexEnum::ColumnIndexFileName, 120);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(TableWidgetColumnIndexEnum::ColumnIndexNum,QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(TableWidgetColumnIndexEnum::ColumnIndexFileName,QHeaderView::Interactive);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(TableWidgetColumnIndexEnum::ColumnIndexFilePath,QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(TableWidgetColumnIndexEnum::ColumnIndexFileSize,QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(TableWidgetColumnIndexEnum::ColumnIndexFormat,QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(TableWidgetColumnIndexEnum::ColumnIndexRTCMTime,QHeaderView::ResizeToContents);
    ui->tableWidget->setEditTriggers(QTableWidget::NoEditTriggers);

    /** Minimum width in logical pixels (aligned with UI file base geometry 660).
     *  Do NOT multiply by devicePixelRatio() — Qt handles HiDPI scaling. */
    ui->tableWidget->setMinimumWidth(660);
    ui->batchStackedWidget->setCurrentIndex(0);
}

ImportFileDialog::~ImportFileDialog()
{
    delete ui;
}

void ImportFileDialog::importFiles(const QStringList &files)
{
    for(const QString &file : files)
    {
        /** filter Eph file */
        QString suffix = QFileInfo(file).suffix();
        if (QRegExp("\\d*[pncgl]").exactMatch(suffix))
        {
            continue;
        }
        /** add OBS file */
        this->addFileRow(file);
    }
}

const QVector<QCConfigurator::QCConfigRawFileInfo> &ImportFileDialog::getImportFileConfigs() const
{
    return m_importFileConfigs;
}

void ImportFileDialog::addFileRow(const QString &file)
{
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    QTableWidgetItem *numItem = new QTableWidgetItem(QString::number(row));
    ui->tableWidget->setItem(row,ColumnIndexNum,numItem);

    ui->tableWidget->setItem(row,ColumnIndexFileName,new QTableWidgetItem(QFileInfo(file).fileName()));
    ui->tableWidget->item(row,ColumnIndexFileName)->setToolTip(QFileInfo(file).fileName());
    ui->tableWidget->setItem(row,ColumnIndexFilePath,new QTableWidgetItem(file));
    ui->tableWidget->item(row,ColumnIndexFilePath)->setToolTip(file);
    ui->tableWidget->setItem(row,ColumnIndexFileSize,new QTableWidgetItem(this->formatFileSize(QFileInfo(file).size())));

    QCConfigurator::RawFileDataFormatEnum autoFormat = this->getFormatFromSuffix(QFileInfo(file).suffix());
    MyComboBox *formatCombobox = new MyComboBox(this);
    QCConfigurator::addFileDataFormatItems(formatCombobox, true);
    ui->tableWidget->setItem(row,ColumnIndexFormat,new QTableWidgetItem(""));
    ui->tableWidget->setCellWidget(row,ColumnIndexFormat,formatCombobox);
    connect(formatCombobox,&MyComboBox::currentTextChanged,[row,this](const QString &text){
        if(isRtcmTimeFormatName(text))
        {
            QDateTimeEdit *edit = qobject_cast<QDateTimeEdit*>(ui->tableWidget->cellWidget(row,ColumnIndexRTCMTime));
            if(edit)
            {
                edit->setEnabled(true);
            }
        }
        else
        {
            QDateTimeEdit *edit = qobject_cast<QDateTimeEdit*>(ui->tableWidget->cellWidget(row,ColumnIndexRTCMTime));
            if(edit)
            {
                edit->setEnabled(false);
            }
        }

    });

    ui->tableWidget->setItem(row,ColumnIndexRTCMTime,new QTableWidgetItem(""));
    QDateTimeEdit *rtcmTimeEdit = new MyDateTimeEdit(this);
    rtcmTimeEdit->setCalendarPopup(true);
    rtcmTimeEdit->setDateTime(QDateTime::currentDateTime());
    ui->tableWidget->setCellWidget(row,ColumnIndexRTCMTime,rtcmTimeEdit);

    formatCombobox->setCurrentText(QCConfigurator::getFileDataFormatName(autoFormat));
    if(0 == row)
    {
        ui->inputFormatComboBox->setCurrentText(formatCombobox->currentText());
    }
}

QString ImportFileDialog::formatFileSize(qint64 fileSize)
{
    const qint64 kb = 1024;
    const qint64 mb = kb * 1024;
    const qint64 gb = mb * 1024;

    if (fileSize >= gb) {
        return QString::number(fileSize / static_cast<double>(gb), 'f', 1) + " GB";
    } else if (fileSize >= mb) {
        double size = fileSize / static_cast<double>(mb);
        return QString::number(size, 'f', size < 10 ? 1 : 0) + " MB";
    } else if (fileSize >= kb) {
        double size = fileSize / static_cast<double>(kb);
        return QString::number(size, 'f', size < 10 ? 1 : 0) + " KB";
    }
    return QString::number(fileSize) + " B";
}

QCConfigurator::RawFileDataFormatEnum ImportFileDialog::getFormatFromSuffix(const QString &suffix)
{
    // Shared implementation (observation-only RINEX pattern). Differences vs
    // the previous dialog-local logic: ephemeris suffixes (p/n/c/g/l) are no
    // longer guessed as RINEX (unreachable here - importFiles() skips them
    // first), while multi-digit/plain "o" observation suffixes now are.
    return QCConfigurator::getFormatFromSuffix(suffix);
}

void ImportFileDialog::onImport()
{
    m_importFileConfigs.clear();

    for (int row = 0; row < ui->tableWidget->rowCount(); ++row)
    {
        QCConfigurator::QCConfigRawFileInfo config;
        config.rawFilePath = ui->tableWidget->item(row,ColumnIndexFilePath)->text();
        config.rawFileDataFormat = (QCConfigurator::RawFileDataFormatEnum)qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(row,ColumnIndexFormat))->currentData().toInt();
        if(QCConfigurator::RawFileDataFormatEnum::None == config.rawFileDataFormat)
        {
            MyMessageBox::critical(this,tr("Error"),tr("The file %1 has not been formatted yet.").arg(ui->tableWidget->item(row,ColumnIndexFileName)->text()));
            return;
        }
        if(ui->tableWidget->cellWidget(row,ColumnIndexRTCMTime)->isEnabled())
        {
            config.rtcmTime = qobject_cast<QDateTimeEdit*>(ui->tableWidget->cellWidget(row,ColumnIndexRTCMTime))->dateTime();
        }
        m_importFileConfigs.append(config);
    }
    this->accept();
}

void ImportFileDialog::onApply()
{
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row)
    {
        if(Qt::Checked == ui->tableWidget->item(row,ColumnIndexCheckBox)->checkState())
        {
            qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(row,ColumnIndexFormat))->setCurrentText(ui->inputFormatComboBox->currentText());
            qobject_cast<QDateTimeEdit*>(ui->tableWidget->cellWidget(row,ColumnIndexRTCMTime))->setDateTime(ui->rtcmTimeEdit->dateTime());
        }
    }
}

void ImportFileDialog::onInputFormatComboBoxChanged(const QString &text)
{
    if(isRtcmTimeFormatName(text))
    {
        ui->rtcmTimeEdit->setEnabled(true);
    }
    else
    {
        ui->rtcmTimeEdit->setEnabled(false);
    }
}

bool ImportFileDialog::isRtcmTimeFormatName(const QString &formatName)
{
    return formatName == QCConfigurator::getFileDataFormatName(QCConfigurator::RawFileDataFormatEnum::RTCM2)
            || formatName == QCConfigurator::getFileDataFormatName(QCConfigurator::RawFileDataFormatEnum::RTCM3)
            || formatName == QCConfigurator::getFileDataFormatName(QCConfigurator::RawFileDataFormatEnum::HRC);
}

void ImportFileDialog::onBatchFormatting()
{
    ui->batchStackedWidget->setCurrentIndex(1);
    for (int row = 0; row < ui->tableWidget->rowCount(); row++)
    {
        auto item = ui->tableWidget->takeItem(row, ColumnIndexCheckBox);
        auto newItem = new QTableWidgetItem();
        newItem->setCheckState(Qt::Unchecked);
        ui->tableWidget->setItem(row, ColumnIndexCheckBox, newItem);
        if (item)
        {
            delete item;
        }
    }
    m_tableHeaderView->setCheckable(true);
    m_tableHeaderView->setCheckState(Qt::Unchecked);
    ui->importButton->setEnabled(false);
    ui->tableWidget->setColumnWidth(TableWidgetColumnIndexEnum::ColumnIndexCheckBox, 30);
}

void ImportFileDialog::onCancelBatchFormatting()
{
    ui->batchStackedWidget->setCurrentIndex(0);
    for (int row = 0; row < ui->tableWidget->rowCount(); row++)
    {
        auto item = ui->tableWidget->takeItem(row, ColumnIndexCheckBox);
        auto newItem = new QTableWidgetItem();
        ui->tableWidget->setItem(row, ColumnIndexCheckBox, newItem);
        if (item)
        {
            delete item;
        }
    }
    m_tableHeaderView->setCheckable(false);
    ui->importButton->setEnabled(true);
    ui->tableWidget->setColumnWidth(TableWidgetColumnIndexEnum::ColumnIndexCheckBox, 0);
}

void ImportFileDialog::onCheckAll(bool checked)
{
    for (int row = 0; row < ui->tableWidget->rowCount(); row++)
    {
        auto item = ui->tableWidget->item(row, ColumnIndexCheckBox);
        if (item)
        {
            item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
        }
    }
}

void ImportFileDialog::onTableItemClicked(QTableWidgetItem* item)
{
    if (item)
    {
        if (ColumnIndexCheckBox == item->column())
        {
            int checkedCount = 0;
            for (int row = 0; row < ui->tableWidget->rowCount(); row++)
            {
                auto item = ui->tableWidget->item(row, ColumnIndexCheckBox);
                if (item && item->checkState() == Qt::Checked)
                {
                    checkedCount++;
                }
            }
            if (checkedCount == ui->tableWidget->rowCount())
            {
                m_tableHeaderView->setCheckState(Qt::Checked);
            }
            else if (checkedCount == 0)
            {
                m_tableHeaderView->setCheckState(Qt::Unchecked);
            }
            else
            {
                m_tableHeaderView->setCheckState(Qt::PartiallyChecked);
            }
            
        }
    }
}
