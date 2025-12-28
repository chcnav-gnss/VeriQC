#include "QCObjectTableModel.h"
#include <QColor>
#include "QCResultManager.h"


QCObjectTableModel::QCObjectTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_headers = this->createHeaders();
}

QVariant QCObjectTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if((role != Qt::DisplayRole && role != Qt::ToolTipRole) || orientation != Qt::Horizontal)
    {
        return QVariant();
    }
    if(section >=0 && section < m_headers.size())
    {
        return m_headers[section];
    }
    return QVariant();
}

int QCObjectTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    int count = 0;
    for(const QCObjectTableModel::TableModelItemData &row : m_dataList)
    {
        count++;
        if(row.expanded)
        {
            count += TableModelSystemIndexEnum::SystemCount;
        }
    }
    return count;
}

int QCObjectTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_headers.size();
}

Qt::ItemFlags QCObjectTableModel::flags(const QModelIndex &index) const
{

    Qt::ItemFlags f = QAbstractTableModel::flags(index);
    if(this->isParentRow(index.row()))
    {
        f = f | Qt::ItemIsUserCheckable;
    }
    return f;
}

QVariant QCObjectTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    QPair<int,bool> parentRowInfo = findParentRow(index.row());
    int parentRow = parentRowInfo.first;
    bool isChild = parentRowInfo.second;
    if(-1 == parentRow)
    {
        return QVariant();
    }
    QSharedPointer<QCObject> qcObject = QCObjectManager::getInstance()->getQCObjectByID(m_dataList[parentRow].objectID);
    bool qcExecuted = qcObject->getQCStatus() != QCObject::QCResultStatusEnum::QCStatusNotExecuted;
    if(!qcObject && Qt::DisplayRole == role)
    {
        return QVariant();
    }
    if(isChild)
    {
        int systemIndex = index.row() - getRowByParentRow(parentRow) - 1;
        QString sysName;
        QCResultManager::QCSysStatisticResultData resultData;
        switch (systemIndex)
        {
        case TableModelSystemIndexEnum::SystemIndexGPS:
            sysName = "GPS";
            resultData = qcObject->getQCResultManager()->getSysStatisticResult(QCResultManager::QCSysTypeEnum::QCSysGPS);
            break;
        case TableModelSystemIndexEnum::SystemIndexBDS:
            sysName = "BDS";
            resultData = qcObject->getQCResultManager()->getSysStatisticResult(QCResultManager::QCSysTypeEnum::QCSysBDS);
            break;
        case TableModelSystemIndexEnum::SystemIndexGLO:
            sysName = "GLO";
            resultData = qcObject->getQCResultManager()->getSysStatisticResult(QCResultManager::QCSysTypeEnum::QCSysGLO);
            break;
        case TableModelSystemIndexEnum::SystemIndexGAL:
            sysName = "GAL";
            resultData = qcObject->getQCResultManager()->getSysStatisticResult(QCResultManager::QCSysTypeEnum::QCSysGAL);
            break;
        case TableModelSystemIndexEnum::SystemIndexQZSS:
            sysName = "QZSS";
            resultData = qcObject->getQCResultManager()->getSysStatisticResult(QCResultManager::QCSysTypeEnum::QCSysQZSS);
            break;
        default:
            return QVariant();
        }

        if(Qt::DisplayRole == role)
        {
            if(resultData.useRate > 0)
            {
                switch (index.column())
                {
                case TABLE_MODEL_COLUMN_INDEX_SYS:
                    return sysName;
                case TABLE_MODEL_COLUMN_INDEX_LOSS_RATE:
                    return "--";
                case TABLE_MODEL_COLUMN_INDEX_USE_RATE:
                    return qcExecuted ? QString("%1%").arg(resultData.useRate,0,'f',1) : "--";
                case TABLE_MODEL_COLUMN_INDEX_CJR:
                    return this->getDataStr(qcExecuted,resultData.cycleJumpRatio,0);
                case TABLE_MODEL_COLUMN_INDEX_MP1:
                    return this->getDataStr(qcExecuted,resultData.MP[QCResultManager::UiComplexMP1],3);
                case TABLE_MODEL_COLUMN_INDEX_MP2:
                    return this->getDataStr(qcExecuted,resultData.MP[QCResultManager::UiComplexMP2],3);
                case TABLE_MODEL_COLUMN_INDEX_MP5:
                    return this->getDataStr(qcExecuted,resultData.MP[QCResultManager::UiComplexMP5],3);
                case TABLE_MODEL_COLUMN_INDEX_MP6:
                    return this->getDataStr(qcExecuted,resultData.MP[QCResultManager::UiComplexMP6],3);
                case TABLE_MODEL_COLUMN_INDEX_MP7:
                    return this->getDataStr(qcExecuted,resultData.MP[QCResultManager::UiComplexMP7],3);
                case TABLE_MODEL_COLUMN_INDEX_MP8:
                    return this->getDataStr(qcExecuted,resultData.MP[QCResultManager::UiComplexMP8],3);
                case TABLE_MODEL_COLUMN_INDEX_SNR1:
                    return this->getDataStr(qcExecuted,resultData.SNR[QCResultManager::UiComplexMP1],1);
                case TABLE_MODEL_COLUMN_INDEX_SNR2:
                    return this->getDataStr(qcExecuted,resultData.SNR[QCResultManager::UiComplexMP2],1);
                case TABLE_MODEL_COLUMN_INDEX_SNR5:
                    return this->getDataStr(qcExecuted,resultData.SNR[QCResultManager::UiComplexMP5],1);
                case TABLE_MODEL_COLUMN_INDEX_SNR6:
                    return this->getDataStr(qcExecuted,resultData.SNR[QCResultManager::UiComplexMP6],1);
                case TABLE_MODEL_COLUMN_INDEX_SNR7:
                    return this->getDataStr(qcExecuted,resultData.SNR[QCResultManager::UiComplexMP7],1);
                case TABLE_MODEL_COLUMN_INDEX_SNR8:
                    return this->getDataStr(qcExecuted,resultData.SNR[QCResultManager::UiComplexMP8],1);
                default:
                    break;
                }
            }
            else
            {
                switch (index.column())
                {
                case TABLE_MODEL_COLUMN_INDEX_SYS:
                    return sysName;
                case TABLE_MODEL_COLUMN_INDEX_LOSS_RATE:
                case TABLE_MODEL_COLUMN_INDEX_USE_RATE:
                case TABLE_MODEL_COLUMN_INDEX_CJR:
                case TABLE_MODEL_COLUMN_INDEX_MP1:
                case TABLE_MODEL_COLUMN_INDEX_MP2:
                case TABLE_MODEL_COLUMN_INDEX_MP5:
                case TABLE_MODEL_COLUMN_INDEX_MP6:
                case TABLE_MODEL_COLUMN_INDEX_MP7:
                case TABLE_MODEL_COLUMN_INDEX_MP8:
                case TABLE_MODEL_COLUMN_INDEX_SNR1:
                case TABLE_MODEL_COLUMN_INDEX_SNR2:
                case TABLE_MODEL_COLUMN_INDEX_SNR5:
                case TABLE_MODEL_COLUMN_INDEX_SNR6:
                case TABLE_MODEL_COLUMN_INDEX_SNR7:
                case TABLE_MODEL_COLUMN_INDEX_SNR8:
                    return "--";
                default:
                    break;
                }
            }

        }

    }
    else
    {
        QCResultManager::QCSysStatisticResultData resultData = qcObject->getQCResultManager()->getSysStatisticResult(QCResultManager::QCSysTypeEnum::QCSysComplex);
        QCObject::QCResultStatusEnum status = qcObject->getQCStatus();
        if(Qt::DisplayRole == role)
        {
            switch (index.column())
            {
            case TABLE_MODEL_COLUMN_INDEX_NUM:
                return parentRow + 1;
            case TABLE_MODEL_COLUMN_INDEX_NAME:
                return qcObject->getName();
            case TABLE_MODEL_COLUMN_INDEX_INPUT_FORMAT:
                return QCConfigurator::getFileDataFormatName(qcObject->getQCConfigurator()->getRawFileInfo().rawFileDataFormat);
            case TABLE_MODEL_COLUMN_INDEX_QC:
                if(QCObject::QCResultStatusEnum::QCStatusPassed == status)
                {
                    return tr("Pass");
                }
                else if(QCObject::QCResultStatusEnum::QCStatusFailed == status)
                {
                    return tr("Fail");
                }
                else
                {
                    return "--";
                }
            case TABLE_MODEL_COLUMN_INDEX_SYS:
                return QString("ALL (%1)").arg(m_dataList[parentRow].expanded ? "-":"+");
            case TABLE_MODEL_COLUMN_INDEX_LOSS_RATE:
                return qcExecuted ? QString("%1%").arg(qcObject->getQCResultManager()->getAllSysLossRate(),0,'f',1) : "--";
            case TABLE_MODEL_COLUMN_INDEX_USE_RATE:
                return qcExecuted && (resultData.useRate > 0) ? QString("%1%").arg(resultData.useRate,0,'f',1) : "--";
            case TABLE_MODEL_COLUMN_INDEX_CJR:
                return this->getDataStr(qcExecuted,resultData.cycleJumpRatio,0);
            case TABLE_MODEL_COLUMN_INDEX_MP1:
                return this->getDataStr(qcExecuted,resultData.MP[QCResultManager::UiComplexMP1],3);
            case TABLE_MODEL_COLUMN_INDEX_MP2:
               return this->getDataStr(qcExecuted,resultData.MP[QCResultManager::UiComplexMP2],3);
            case TABLE_MODEL_COLUMN_INDEX_MP5:
               return this->getDataStr(qcExecuted,resultData.MP[QCResultManager::UiComplexMP5],3);
            case TABLE_MODEL_COLUMN_INDEX_MP6:
               return this->getDataStr(qcExecuted,resultData.MP[QCResultManager::UiComplexMP6],3);
            case TABLE_MODEL_COLUMN_INDEX_MP7:
               return this->getDataStr(qcExecuted,resultData.MP[QCResultManager::UiComplexMP7],3);
            case TABLE_MODEL_COLUMN_INDEX_MP8:
               return this->getDataStr(qcExecuted,resultData.MP[QCResultManager::UiComplexMP8],3);
            case TABLE_MODEL_COLUMN_INDEX_SNR1:
               return this->getDataStr(qcExecuted,resultData.SNR[QCResultManager::UiComplexMP1],1);
            case TABLE_MODEL_COLUMN_INDEX_SNR2:
               return this->getDataStr(qcExecuted,resultData.SNR[QCResultManager::UiComplexMP2],1);
            case TABLE_MODEL_COLUMN_INDEX_SNR5:
               return this->getDataStr(qcExecuted,resultData.SNR[QCResultManager::UiComplexMP5],1);
            case TABLE_MODEL_COLUMN_INDEX_SNR6:
               return this->getDataStr(qcExecuted,resultData.SNR[QCResultManager::UiComplexMP6],1);
            case TABLE_MODEL_COLUMN_INDEX_SNR7:
               return this->getDataStr(qcExecuted,resultData.SNR[QCResultManager::UiComplexMP7],1);
            case TABLE_MODEL_COLUMN_INDEX_SNR8:
               return this->getDataStr(qcExecuted,resultData.SNR[QCResultManager::UiComplexMP8],1);
            default:
                break;
            }
        }
        else if(Qt::CheckStateRole == role)
        {
            if(TABLE_MODEL_COLUMN_INDEX_CHECK_BOX == index.column())
            {
                if(parentRow != -1)
                {
                    return m_dataList[parentRow].checked ? Qt::Checked : Qt::Unchecked;
                }
            }
        }
        else if (Qt::ForegroundRole == role && QCObject::QCResultStatusEnum::QCStatusFailed == status)
        {
            const QCObject::QCCheckStatusInfoStruct &checkStatusInfo = qcObject->getQCCheckStatusInfo();
            if(!checkStatusInfo.useRatePass && TABLE_MODEL_COLUMN_INDEX_USE_RATE == index.column())
            {
                return QColor(Qt::red);
            }
            if(!checkStatusInfo.cycleJumpRatioPass && TABLE_MODEL_COLUMN_INDEX_CJR == index.column())
            {
                return QColor(Qt::red);
            }
            for (int i = 0; i < UI_MP_MAX_FREQ_NUM; ++i)
            {
                if(!checkStatusInfo.mpPass[i] && (TABLE_MODEL_COLUMN_INDEX_MP1 + i) == index.column())
                {
                    return QColor(Qt::red);
                }
                if(!checkStatusInfo.snrPass[i] && (TABLE_MODEL_COLUMN_INDEX_SNR1 + i) == index.column())
                {
                    return QColor(Qt::red);
                }
            }
        }
        else if(Qt::ToolTipRole == role)
        {
            if(TABLE_MODEL_COLUMN_INDEX_NAME == index.column())
            {
                return qcObject->getName();
            }
        }
    }


    // FIXME: Implement me!
    return QVariant();
}

bool QCObjectTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(TABLE_MODEL_COLUMN_INDEX_SYS == index.column() && Qt::EditRole == role)
    {
        QPair<int,bool> parentRowInfo = findParentRow(index.row());
        int parentRow = parentRowInfo.first;
        bool isChild = parentRowInfo.second;
        if(parentRow != -1 && !isChild)
        {
            toggleExpand(parentRow,index);
            return true;
        }
    }
    else if(TABLE_MODEL_COLUMN_INDEX_CHECK_BOX == index.column() && Qt::CheckStateRole == role)
    {
        QPair<int,bool> parentRowInfo = findParentRow(index.row());
        int parentRow = parentRowInfo.first;
        bool isChild = parentRowInfo.second;
        if(parentRow != -1 && !isChild)
        {
            m_dataList[parentRow].checked = value.toBool();
            emit dataChanged(index,index);
            emit updateHeaderCheckState(this->calculateHeaderCheckState());
            return true;
        }

    }
    return false;
}

bool QCObjectTableModel::isParentRow(int row) const
{
    QPair<int,bool> parentRowInfo = findParentRow(row);
    int parentRow = parentRowInfo.first;
    bool isChild = parentRowInfo.second;
    if(parentRow != -1 && !isChild)
    {
        return true;
    }
    return false;
}

void QCObjectTableModel::addQCObjects(const QVector<int> &objectIDs)
{
    int startRow = rowCount();
    int endRow = rowCount() + objectIDs.size() - 1;
    beginInsertRows(QModelIndex(),startRow,endRow);
    for(int id : objectIDs)
    {
        QSharedPointer<QCObject> qcObject = QCObjectManager::getInstance()->getQCObjectByID(id);
        Q_ASSERT(qcObject);
        QCObjectTableModel::TableModelItemData data;
        data.objectID = id;
        m_dataList.append(data);
    }
    endInsertRows();
    for (int row = startRow; row <= endRow; ++row)
    {
        this->setData(index(row,TABLE_MODEL_COLUMN_INDEX_CHECK_BOX),Qt::Checked,Qt::CheckStateRole);
    }
}

void QCObjectTableModel::removeQCObject(int objectID)
{
    for (int i = 0; i < m_dataList.size(); ++i)
    {
        if(m_dataList[i].objectID == objectID)
        {
            int row = this->getRowByParentRow(i);
            if(m_dataList[i].expanded)
            {

                beginRemoveRows(QModelIndex(),row,row + TableModelSystemIndexEnum::SystemCount);
                m_dataList.remove(i);
                endRemoveRows();
            }
            else
            {
                beginRemoveRows(QModelIndex(),row,row);
                m_dataList.remove(i);
                endRemoveRows();
            }

            break;
        }
    }
}

void QCObjectTableModel::updateQCObject(int objectID)
{
    for (int parentRow = 0; parentRow < m_dataList.count(); ++parentRow)
    {
        if(m_dataList[parentRow].objectID == objectID)
        {
            // refresh rows
            int row = getRowByParentRow(parentRow);
            if(m_dataList[parentRow].expanded)
            {
                emit dataChanged(this->index(row,0),this->index(row + TableModelSystemIndexEnum::SystemCount,columnCount()-1),{Qt::DisplayRole});
            }
            else
            {
                emit dataChanged(this->index(row,0),this->index(row,columnCount()-1),{Qt::DisplayRole});
            }

            break;
        }
    }
}

QVector<int> QCObjectTableModel::getSelectedQcObjects() const
{
    QVector<int> selectedObjs;
    for(const QCObjectTableModel::TableModelItemData &data: m_dataList)
    {
        if(data.checked)
        {
            selectedObjs.append(data.objectID);
        }
    }
    return selectedObjs;
}

QSharedPointer<QCObject> QCObjectTableModel::getQCObject(int row)
{
    QPair<int,bool> parentRowInfo = findParentRow(row);
    int parentRow = parentRowInfo.first;
    if(-1 == parentRow)
    {
        return nullptr;
    }
    return QCObjectManager::getInstance()->getQCObjectByID(m_dataList[parentRow].objectID);
}

void QCObjectTableModel::retranslateUi()
{
    m_headers = this->createHeaders();
}

void QCObjectTableModel::toggleExpand(int parentRow, const QModelIndex &index)
{
    bool &expanded = m_dataList[parentRow].expanded;
    if(expanded)
    {
        beginRemoveRows(QModelIndex(),index.row() + 1,index.row() + TableModelSystemIndexEnum::SystemCount);
        expanded = false;
        endRemoveRows();
    }
    else
    {
        beginInsertRows(QModelIndex(),index.row() + 1,index.row() + TableModelSystemIndexEnum::SystemCount);
        expanded = true;
        endInsertRows();
        emit dataChanged(this->index(index.row() + 1,0),
                         this->index(index.row() + TableModelSystemIndexEnum::SystemCount,this->columnCount() - 1));
    }
}

QPair<int, bool> QCObjectTableModel::findParentRow(int row) const
{
    int current = 0;
    for (int i = 0; i < m_dataList.size(); ++i)
    {
        if(row == current)
        {
            return {i,false};
        }
        if(row > current && row <= current + (m_dataList[i].expanded ? TableModelSystemIndexEnum::SystemCount : 0))
        {
            return {i,true};
        }
        current += 1+(m_dataList[i].expanded ? TableModelSystemIndexEnum::SystemCount : 0);
    }
    return {-1,false};
}

int QCObjectTableModel::getRowByParentRow(int parentRow)const
{
    int row = 0;
    for (int i = 0; i < m_dataList.size(); ++i)
    {
        if(i == parentRow)
        {
            return row;
        }
        row += 1+(m_dataList[i].expanded ? TableModelSystemIndexEnum::SystemCount : 0);
    }
    return -1;
}

void QCObjectTableModel::updateAllCheckedStates(bool checked)
{
    for (TableModelItemData &row : m_dataList)
    {
        row.checked = checked;
    }
    emit dataChanged(index(0,TABLE_MODEL_COLUMN_INDEX_CHECK_BOX), index(rowCount()-1,TABLE_MODEL_COLUMN_INDEX_CHECK_BOX));
}

QString QCObjectTableModel::getDataStr(bool executedQC, double value, int prec)
{
    if(executedQC && (value > 0))
    {
        return QString::number(value,'f',prec);
    }
    return "--";
}

Qt::CheckState QCObjectTableModel::calculateHeaderCheckState()
{
    Qt::CheckState state = Qt::Unchecked;
    int checkedRowCount = 0;
    for (TableModelItemData &row : m_dataList)
    {
        if(row.checked)
        {
            checkedRowCount++;
        }
    }
    if(0 == checkedRowCount)
    {
        state = Qt::Unchecked;
    }
    else if(checkedRowCount == m_dataList.size())
    {
        state = Qt::Checked;
    }
    else
    {
        state = Qt::PartiallyChecked;
    }

    return state;
}

QStringList QCObjectTableModel::createHeaders()
{
    return QStringList()<<""<<QObject::tr("Num")<<QObject::tr("Obs File")<<QObject::tr("Input Format")<<QObject::tr("QC")
                       <<QObject::tr("System")<<QObject::tr("Loss Rate")<<QObject::tr("Use Rate")<<QObject::tr("Cycle Jump Ratio")
                       <<QString("MP1(%1)").arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP1))
                       <<QString("MP2(%1)").arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP2))
                       <<QString("MP5(%1)").arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP5))
                       <<QString("MP6(%1)").arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP6))
                       <<QString("MP7(%1)").arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP7))
                       <<QString("MP8(%1)").arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP8))
                       <<QString("SNR1(%1)").arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP1))
                       <<QString("SNR2(%1)").arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP2))
                       <<QString("SNR5(%1)").arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP5))
                       <<QString("SNR6(%1)").arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP6))
                       <<QString("SNR7(%1)").arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP7))
                       <<QString("SNR8(%1)").arg(QCResultManager::getUIComplexFreqsName(QCResultManager::UiComplexMP8));
}
