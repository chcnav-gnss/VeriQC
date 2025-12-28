/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Module
 * *-
 * @file ImportFileDialog.h
 * @author CHC
 * @date 2025-09-28
 * @brief dialog for import files
 * 
**************************************************************************/
#ifndef IMPORTFILEDIALOG_H
#define IMPORTFILEDIALOG_H

#include "Components/MyDialog.h"
#include "QCConfigurator.h"

namespace Ui {
class ImportFileDialog;
}

class MyHeaderView;
class QTableWidgetItem;
class ImportFileDialog : public MyDialog
{
    Q_OBJECT
public:
    enum TableWidgetColumnIndexEnum
    {
        ColumnIndexCheckBox,
        ColumnIndexNum,
        ColumnIndexFileName,
        ColumnIndexFilePath,
        ColumnIndexFileSize,
        ColumnIndexFormat,
        ColumnIndexRTCMTime,
    };
public:
    explicit ImportFileDialog(QWidget *parent = nullptr);
    ~ImportFileDialog();
    void importFiles(const QStringList &files);
    const QVector<QCConfigurator::QCConfigRawFileInfo> &getImportFileConfigs()const;
protected:
    void addFileRow(const QString &file);
    static QString formatFileSize(qint64 fileSize);
    static QCConfigurator::RawFileDataFormatEnum getFormatFromSuffix(const QString& suffix);
    void onImport();
    void onApply();
    void onInputFormatComboBoxChanged(const QString &text);
    void onBatchFormatting();
    void onCancelBatchFormatting();
    void onCheckAll(bool checked);
    void onTableItemClicked(QTableWidgetItem* item);
private:
    Ui::ImportFileDialog *ui;
    QVector<QCConfigurator::QCConfigRawFileInfo> m_importFileConfigs;
    MyHeaderView* m_tableHeaderView = nullptr;
};

#endif // IMPORTFILEDIALOG_H
