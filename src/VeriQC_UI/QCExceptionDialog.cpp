#include "QCExceptionDialog.h"
#include "ui_QCExceptionDialog.h"
#include "QCObject.h"
#include "QCResultManager.h"
#include "VeriQC.h"

QCExceptionDialog::QCExceptionDialog(const QVector<QSharedPointer<QCObject>> &qcObjects,
                                     QWidget *parent) :
    MyDialog(parent),
    ui(new Ui::QCExceptionDialog)
{
    ui->setupUi(this);

    ui->exceptionTreeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->exceptionTreeWidget->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->exceptionTreeWidget->setRootIsDecorated(true);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

    populateTable(qcObjects);
}

QCExceptionDialog::~QCExceptionDialog()
{
    delete ui;
}

bool QCExceptionDialog::hasAnyExceptions(const QVector<QSharedPointer<QCObject>> &qcObjects)
{
    for (const auto &obj : qcObjects)
    {
        if (obj.isNull())
        {
            continue;
        }
        const QC_RESULT_T &result = obj->getQCResultManager()->getQCResult();

        /** Check file-level exceptions for rover (0) and base (1) */
        for (int i = 0; i < QC_MAX_NUM_OBS_FILE; i++)
        {
            if (result.FileException[i].Code != QC_EXCEPTION_OK)
            {
                return true;
            }
        }

        /** Check system-level exceptions */
        for (int i = 0; i < QC_MAX_NUM_OBS_FILE; i++)
        {
            for (int j = 0; j < QC_MAX_NUM_SYS; j++)
            {
                if (result.SysException[i][j].Code != QC_EXCEPTION_OK)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

void QCExceptionDialog::populateTable(const QVector<QSharedPointer<QCObject>> &qcObjects)
{
    int fileLevelCount = 0;
    int sysLevelCount = 0;
    int fileCount = 0;

    for (const auto &obj : qcObjects)
    {
        if (obj.isNull())
        {
            continue;
        }

        const QC_RESULT_T &result = obj->getQCResultManager()->getQCResult();
        bool hasFileException = false;
        bool hasSysException = false;

        /** Check if this file has any exception */
        for (int i = 0; i < QC_MAX_NUM_OBS_FILE; i++)
        {
            if (result.FileException[i].Code != QC_EXCEPTION_OK)
            {
                hasFileException = true;
            }
            for (int j = 0; j < QC_MAX_NUM_SYS; j++)
            {
                if (result.SysException[i][j].Code != QC_EXCEPTION_OK)
                {
                    hasSysException = true;
                }
            }
        }

        if (!hasFileException && !hasSysException)
        {
            continue;
        }

        fileCount++;
        QString fileName = obj->getName();
        QTreeWidgetItem *fileItem = new QTreeWidgetItem();
        fileItem->setText(0, fileName);
        ui->exceptionTreeWidget->addTopLevelItem(fileItem);

        /** Add file-level exception items first */
        for (int i = 0; i < QC_MAX_NUM_OBS_FILE; i++)
        {
            if (result.FileException[i].Code != QC_EXCEPTION_OK)
            {
                fileLevelCount++;
                QTreeWidgetItem *child = new QTreeWidgetItem();
                child->setText(0, tr("[File]"));
                child->setText(1, exceptionCodeToString(result.FileException[i].Code, result.FileException[i]));
                fileItem->addChild(child);
            }
        }

        /** Add system-level exception items */
        for (int i = 0; i < QC_MAX_NUM_OBS_FILE; i++)
        {
            for (int j = 0; j < QC_MAX_NUM_SYS; j++)
            {
                if (result.SysException[i][j].Code != QC_EXCEPTION_OK)
                {
                    sysLevelCount++;
                    QTreeWidgetItem *child = new QTreeWidgetItem();
                    child->setText(0, tr("[System: %1]").arg(systemIndexToName(j)));
                    child->setText(1, exceptionCodeToString(result.SysException[i][j].Code, result.SysException[i][j]));
                    fileItem->addChild(child);
                }
            }
        }

        fileItem->setExpanded(true);
    }

    /** Update summary label */
    QString summary;
    if (fileCount == 0)
    {
        summary = tr("No QC exceptions detected.");
    }
    else
    {
        summary = tr("%1 file-level and %2 system-level exceptions detected across %3 file(s).")
                      .arg(fileLevelCount)
                      .arg(sysLevelCount)
                      .arg(fileCount);
    }
    ui->summaryLabel->setText(summary);
}

QString QCExceptionDialog::exceptionCodeToString(int code, const QC_EXCEPTION_INFO_T &info)
{
    switch (code)
    {
    case QC_EXCEPTION_UNSUPPORTED_FORMAT:
        return tr("The file format is not supported.");
    case QC_EXCEPTION_OBS_FILE_OPEN_FAILED:
        return tr("Failed to open observation file.");
    case QC_EXCEPTION_NAV_FILE_MISSING:
        return tr("Navigation/ephemeris file is missing.");
    case QC_EXCEPTION_PARSE_FAILED:
        return tr("File parsing failed.");
    case QC_EXCEPTION_PARSE_HEADER_FAILED:
        return tr("File header parsing failed.");
    case QC_EXCEPTION_PARSE_EPOCH_FAILED:
        return tr("Epoch parsing failed.");
    case QC_EXCEPTION_PARTIAL_SYS_FAILED:
        return tr("QC processing failed for one or more GNSS systems.");
    case QC_EXCEPTION_SPP_ALL_FAILED:
        return tr("All SPP solutions failed.");
    case QC_EXCEPTION_SYS_NAV_UNMATCHED:
    {
        /** Append clarifying flags for system-level nav unmatched */
        QString desc = tr("Observation data exists but no matching navigation data found.");
        if (!info.IsHaveEph)
        {
            desc += tr(" (No ephemeris injected for this system.)");
        }
        return desc;
    }
    case QC_EXCEPTION_SYS_SPP_UNSOLVED:
    {
        /** Append clarifying flags for system-level SPP unsolved */
        QString desc = tr("Observation and navigation data exist but SPP is still unsolved.");
        if (info.IsAllEpochSPPUnsolved)
        {
            desc += tr(" (All epochs SPP unsolved for this system.)");
        }
        return desc;
    }
    default:
        return tr("Unknown exception (code: %1).").arg(code);
    }
}

QString QCExceptionDialog::systemIndexToName(int sysIndex)
{
    switch (sysIndex)
    {
    case SYS_INDEX_GPS:  return tr("GPS");
    case SYS_INDEX_SBAS: return tr("SBAS");
    case SYS_INDEX_GLO:  return tr("GLO");
    case SYS_INDEX_GAL:  return tr("GAL");
    case SYS_INDEX_QZSS: return tr("QZSS");
    case SYS_INDEX_BDS:  return tr("BDS");
    case SYS_INDEX_XW1:  return tr("XW1");
    default:             return tr("Unknown");
    }
}
