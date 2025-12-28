/**********************************************************************//**
 * 		VeriQC
 * 
 * 		UI Module
 * *-
 * @file QCObjectTableModel.h
 * @author CHC
 * @date 2025-09-28
 * @brief model of main table view
 * 
**************************************************************************/
#ifndef QCOBJECTTABLEMODEL_H
#define QCOBJECTTABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include "QCObjectManager.h"

#define TABLE_MODEL_COLUMN_INDEX_CHECK_BOX      0
#define TABLE_MODEL_COLUMN_INDEX_NUM            1
#define TABLE_MODEL_COLUMN_INDEX_NAME           2
#define TABLE_MODEL_COLUMN_INDEX_INPUT_FORMAT   3
#define TABLE_MODEL_COLUMN_INDEX_QC             4
#define TABLE_MODEL_COLUMN_INDEX_SYS            5
#define TABLE_MODEL_COLUMN_INDEX_LOSS_RATE      6
#define TABLE_MODEL_COLUMN_INDEX_USE_RATE       7
#define TABLE_MODEL_COLUMN_INDEX_CJR            8
#define TABLE_MODEL_COLUMN_INDEX_MP1            9
#define TABLE_MODEL_COLUMN_INDEX_MP2            10
#define TABLE_MODEL_COLUMN_INDEX_MP5            11
#define TABLE_MODEL_COLUMN_INDEX_MP6            12
#define TABLE_MODEL_COLUMN_INDEX_MP7            13
#define TABLE_MODEL_COLUMN_INDEX_MP8            14
#define TABLE_MODEL_COLUMN_INDEX_SNR1           15
#define TABLE_MODEL_COLUMN_INDEX_SNR2           16
#define TABLE_MODEL_COLUMN_INDEX_SNR5           17
#define TABLE_MODEL_COLUMN_INDEX_SNR6           18
#define TABLE_MODEL_COLUMN_INDEX_SNR7           19
#define TABLE_MODEL_COLUMN_INDEX_SNR8           20

class QCObjectTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum TableModelSystemIndexEnum
    {
        SystemIndexGPS,
        SystemIndexBDS,
        SystemIndexGLO,
        SystemIndexGAL,
        SystemIndexQZSS,

        SystemCount,
    };

    struct TableModelItemData
    {
        int objectID = 0;
        bool expanded = false;  // is user expanded for showing each system rows, default is hidden
        bool checked = false;
    };

public:
    explicit QCObjectTableModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    bool isParentRow(int row)const;
    void addQCObjects(const QVector<int> &objectIDs);
    void removeQCObject(int objectID);
    void updateQCObject(int objectID);
    QVector<int> getSelectedQcObjects() const;
    QSharedPointer<QCObject> getQCObject(int row);
    void retranslateUi();
    void updateAllCheckedStates(bool checked);
signals:
    void updateHeaderCheckState(Qt::CheckState state);
protected:
    void toggleExpand(int parentRow, const QModelIndex &index);
    QPair<int, bool> findParentRow(int row)const;
    int getRowByParentRow(int parentRow)const;
    static QString getDataStr(bool executedQC,double value,int prec = 6);
    Qt::CheckState calculateHeaderCheckState();
    QStringList createHeaders();
private:
    QVector<QCObjectTableModel::TableModelItemData> m_dataList;
    QStringList m_headers;
};

#endif // QCOBJECTTABLEMODEL_H
