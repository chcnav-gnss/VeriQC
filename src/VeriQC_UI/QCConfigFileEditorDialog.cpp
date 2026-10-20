/**********************************************************************//**
 * 		VeriQC
 *
 * 		UI Module
 *-
 * @file QCConfigFileEditorDialog.cpp
 * @author CHC
 * @date 2026-09-02
 * @brief Modal editor for the engine GNSS QC config file
**************************************************************************/
#include "QCConfigFileEditorDialog.h"
#include "ui_QCConfigFileEditorDialog.h"

#include "Components/MyMessageBox.h"

#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QFileDialog>
#include <QFont>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QToolButton>

using namespace QCConfigFileModel;

namespace {

/**< Collapsible section header: arrow direction and panel visibility follow
 *  the check state of the tool button. */
void connectSectionToggle(QToolButton *button, QWidget *panel)
{
    QObject::connect(button, &QToolButton::toggled, panel, [button, panel](bool checked) {
        button->setArrowType(checked ? Qt::DownArrow : Qt::RightArrow);
        panel->setVisible(checked);
    });
}

/**< Splits a comma-separated double list; *ok reports parse failures. */
QVector<double> parseDoubleList(const QString &text, bool *ok)
{
    QVector<double> values;
    *ok = true;
    const QStringList items = text.split(QLatin1Char(','), QString::SkipEmptyParts);
    for (const QString &item : items)
    {
        bool valid = false;
        const double v = item.trimmed().toDouble(&valid);
        if (!valid)
        {
            *ok = false;
            return values;
        }
        values << v;
    }
    return values;
}

/**< Splits a comma-separated integer list; *ok reports parse failures. */
QVector<int> parseIntList(const QString &text, bool *ok)
{
    QVector<int> values;
    *ok = true;
    const QStringList items = text.split(QLatin1Char(','), QString::SkipEmptyParts);
    for (const QString &item : items)
    {
        bool valid = false;
        const int v = item.trimmed().toInt(&valid);
        if (!valid)
        {
            *ok = false;
            return values;
        }
        values << v;
    }
    return values;
}

} // namespace

QCConfigFileEditorDialog::QCConfigFileEditorDialog(QWidget *parent)
    : MyDialog(parent)
    , ui(new Ui::QCConfigFileEditorDialog)
{
    ui->setupUi(this);
    setMinimumWidth(640); /**< Logical pixels under High-DPI PassThrough, same contract as QCConfigDialog. */

    ui->modeComboBox->clear();
    ui->modeComboBox->addItem(tr("-1: None"), -1);
    ui->modeComboBox->addItem(tr("0: Zero baseline"), 0);
    ui->modeComboBox->addItem(tr("1: Diff sat and epoch"), 1);
    ui->modeComboBox->addItem(tr("2: Short baseline"), 2);
    ui->modeComboBox->addItem(tr("3: Triple diff epoch"), 3);

    ui->rawDataTypeComboBox->clear();
    ui->rawDataTypeComboBox->addItem(tr("-1: Auto detect"), -1);
    ui->rawDataTypeComboBox->addItem(tr("0: RTCM2"), 0);
    ui->rawDataTypeComboBox->addItem(tr("1: RTCM3"), 1);
    ui->rawDataTypeComboBox->addItem(tr("11: RINEX"), 11);
    ui->rawDataTypeComboBox->addItem(tr("37: HRCX"), 37);
    ui->rawDataTypeComboBox->addItem(tr("38: HRC"), 38);
    ui->rawDataTypeComboBox->addItem(tr("51: RawMsg ASCII"), 51);
    ui->rawDataTypeComboBox->addItem(tr("52: RawMsg BIN"), 52);
    ui->rawDataTypeComboBox->addItem(tr("55: UNRAWMSGB"), 55);
    ui->rawDataTypeComboBox->addItem(tr("56: CHCRAWMSGB"), 56);
    ui->rawDataTypeComboBox->addItem(tr("101: KMD RawMsg ASCII"), 101);
    ui->rawDataTypeComboBox->addItem(tr("102: KMD RawMsg BIN"), 102);
    ui->rawDataTypeComboBox->addItem(tr("103: RTKLOGA"), 103);
    ui->rawDataTypeComboBox->addItem(tr("104: RTKLOGB"), 104);
    ui->rawDataTypeComboBox->addItem(tr("105: CLAS"), 105);

    ui->mpMethodComboBox->clear();
    ui->mpMethodComboBox->addItem(tr("0: Fixed 50-epoch"), 0);
    ui->mpMethodComboBox->addItem(tr("1: Filter"), 1);

    m_sysChecks[0] = ui->sysGPSChk;
    m_sysChecks[1] = ui->sysSBASChk;
    m_sysChecks[2] = ui->sysGLOChk;
    m_sysChecks[3] = ui->sysGALChk;
    m_sysChecks[4] = ui->sysQZSChk;
    m_sysChecks[5] = ui->sysBDSChk;
    m_sysChecks[6] = ui->sysLEOChk;

    buildThresholdRows();

    /**< Expert review 2026-09: fields the product surface does not support stay
     *  out of sight while the widgets (and the model behind them) keep working,
     *  so an opened file still round-trips these keys verbatim and a new file
     *  writes the model defaults (Mode -1, MPMethod 0, IsThreshold* true,
     *  MpCarrier 0.005, HeightToReflector 0). */
    ui->modeLabel->hide();
    ui->modeComboBox->hide();
    ui->mpMethodLabel->hide();
    ui->mpMethodComboBox->hide();
    ui->thresholdSNRChk->hide();
    ui->thresholdMPChk->hide();
    ui->heightToReflectorLabel->hide();
    ui->heightToReflectorSpinBox->hide();
    ui->mpCarrierLabel->hide();
    ui->mpCarrierSpinBox->hide();
    /**< The baseline group is hidden as a whole (button + panel); the widgets
     *  keep loading/saving BaseTrueXYZ/RoverTrueXYZ/IsInputTrueXYZ/LLI keys. */
    ui->baselineToolButton->hide();
    ui->baselineWidget->hide();

    connectSectionToggle(ui->commonToolButton, ui->commonWidget);
    connectSectionToggle(ui->outputToolButton, ui->outputWidget);
    connectSectionToggle(ui->thresholdToolButton, ui->thresholdWidget);
    connectSectionToggle(ui->baselineToolButton, ui->baselineWidget);

    connect(ui->rtcmTimeEnabledCheckBox, &QCheckBox::toggled, ui->rtcmTimeEdit, &QDateTimeEdit::setEnabled);
    connect(ui->startTimeEnabledCheckBox, &QCheckBox::toggled, ui->startDateTimeEdit, &QDateTimeEdit::setEnabled);
    connect(ui->endTimeEnabledCheckBox, &QCheckBox::toggled, ui->endDateTimeEdit, &QDateTimeEdit::setEnabled);
    /**< An unchecked system never feeds the engine, so its threshold row is grayed out
     *   (the values themselves are kept verbatim for file fidelity). Row 7 (NavIC)
     *   has no Sys letter and no checkbox, so it stays enabled unconditionally. */
    for (int i = 0; i < 7; ++i)
    {
        connect(m_sysChecks[i], &QCheckBox::toggled, this, [this, i](bool checked) {
            m_mpMaxEdits[i]->setEnabled(checked);
            m_snrMinEdits[i]->setEnabled(checked);
            m_numSnrEdits[i]->setEnabled(checked);
        });
    }
    connect(ui->rbEphCustom, &QRadioButton::toggled, ui->leEphFilePaths, &QLineEdit::setEnabled);
    connect(ui->rbEphCustom, &QRadioButton::toggled, ui->btnEphBrowse, &QToolButton::setEnabled);

    connect(ui->btnEphBrowse, &QToolButton::clicked, this, [this]() {
        const QString file = QFileDialog::getOpenFileName(this, tr("Select RINEX Ephemeris/Nav File"),
                                                          QString(), tr("All Files (*)"));
        if (!file.isEmpty()) ui->leEphFilePaths->setText(file);
    });

    connect(ui->btnNew, &QPushButton::clicked, this, &QCConfigFileEditorDialog::onNew);
    connect(ui->btnOpen, &QPushButton::clicked, this, &QCConfigFileEditorDialog::onOpen);
    connect(ui->btnSave, &QPushButton::clicked, this, &QCConfigFileEditorDialog::onSave);
    connect(ui->btnSaveAs, &QPushButton::clicked, this, &QCConfigFileEditorDialog::onSaveAs);
    connect(ui->btnClose, &QPushButton::clicked, this, &QCConfigFileEditorDialog::reject);

    connectDirtyWatchers();

    /**< Remember the standard item counts so fillFrom can strip dynamic
     *  entries appended for a previously loaded file. */
    m_baseComboCounts.insert(ui->modeComboBox, ui->modeComboBox->count());
    m_baseComboCounts.insert(ui->rawDataTypeComboBox, ui->rawDataTypeComboBox->count());
    m_baseComboCounts.insert(ui->mpMethodComboBox, ui->mpMethodComboBox->count());

    fillFrom(defaultGnssCfgFileData());
    m_dirty = false;
    updateTitle();

    /**< Wheel inside the form must scroll the page, never change control values:
     *   QComboBox/QAbstractSpinBox base classes consume wheel to change values,
     *   so the eventFilter below intercepts every child's wheel event, forwards
     *   it to the scroll area's viewport and consumes it (the MyNoWheel* spin
     *   boxes ignore wheel in their own wheelEvent as an extra safeguard). */
    for (QWidget *w : ui->scrollArea->findChildren<QWidget*>())
    {
        w->installEventFilter(this);
    }
}

QCConfigFileEditorDialog::~QCConfigFileEditorDialog()
{
    delete ui;
}

bool QCConfigFileEditorDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Wheel)
    {
        QWidget *w = qobject_cast<QWidget *>(watched);
        /**< Forward wheels raised on form contents inside the viewport only.
         *   Qt redirects a viewport wheel to the scrollbars through a real
         *   QApplication::sendEvent (QAbstractScrollArea::wheelEvent), and the
         *   scrollbars are filtered here too, so forwarding for any widget
         *   outside the viewport would re-enter this filter and recurse on the
         *   UI thread until the stack overflows (crashes seen in the field).
         *   The viewport itself is skipped explicitly (never rely on
         *   isAncestorOf() semantics for self-exclusion); popup windows
         *   (combo lists, the date-time calendar) fail both the ancestor and
         *   window checks and keep scrolling themselves. */
        QWidget *viewport = ui->scrollArea->viewport();
        if (w && w != viewport && viewport->isAncestorOf(w) && w->window() == this)
        {
            QApplication::sendEvent(viewport, event);
            return true;
        }
    }
    return QDialog::eventFilter(watched, event);
}

void QCConfigFileEditorDialog::buildThresholdRows()
{
    QGridLayout *grid = ui->thresholdGridLayout;
    int row = 0;
    auto addHeader = [grid, &row](int column, const QString &text) {
        QLabel *header = new QLabel(text, grid->parentWidget());
        QFont headerFont = header->font();
        headerFont.setBold(true);
        header->setFont(headerFont);
        grid->addWidget(header, row, column);
    };
    addHeader(0, tr("System"));
    addHeader(1, tr("MaxMP Rms [m] (comma separated)"));
    addHeader(2, tr("MinSNR [dBHz] (comma separated)"));
    ++row;
    for (int i = 0; i < 8; ++i)
    {
        /**< Row 7 is NavIC, which has no Sys letter (systemLetter would read
         *  past kSysLetters), so its label uses a letter-free variant. */
        const QString label = (i < 7)
                                  ? tr("%1 (%2, %3 freqs)")
                                        .arg(systemLabel(i), QString(systemLetter(i)),
                                             QString::number(systemFreqCount(i)))
                                  : tr("%1 (no Sys letter, %2 freqs)")
                                        .arg(systemLabel(i))
                                        .arg(QString::number(systemFreqCount(i)));
        grid->addWidget(new QLabel(label, grid->parentWidget()), row, 0);

        const QString tip = tr("Exactly %1 values for %2, comma separated: %3")
                                .arg(systemFreqCount(i)).arg(systemLabel(i), systemFreqNames(i));
        m_mpMaxEdits[i] = new QLineEdit(grid->parentWidget());
        m_mpMaxEdits[i]->setToolTip(tip);
        m_mpMaxEdits[i]->setMinimumWidth(110);
        m_snrMinEdits[i] = new QLineEdit(grid->parentWidget());
        m_snrMinEdits[i]->setToolTip(tip);
        m_snrMinEdits[i]->setMinimumWidth(110);
        m_numSnrEdits[i] = new QLineEdit(grid->parentWidget());
        m_numSnrEdits[i]->setToolTip(tip);
        m_numSnrEdits[i]->setMinimumWidth(110);
        grid->addWidget(m_mpMaxEdits[i], row, 1);
        grid->addWidget(m_snrMinEdits[i], row, 2);
        grid->addWidget(m_numSnrEdits[i], row, 3);
        /**< Hidden after being gridded (a parented, never-gridded widget would
         * pop up at (0,0)): the MinSNR Sat Num column is not part of the
         * product surface, but the edits stay alive so collectTo keeps
         * writing the loaded/numSnrMin defaults verbatim. The grid does not
         * reserve space for hidden widgets, so column 3 collapses. */
        m_numSnrEdits[i]->hide();
        if (i == 7)
        {
            /**< NavIC row: NIC* keys are no longer consumed by the engine;
             *  the row exists only so old files round-trip their values. */
            const QString nicTip = tr("%1 (kept for file compatibility; the engine does not consume these keys)")
                                       .arg(systemFreqNames(i));
            m_mpMaxEdits[i]->setToolTip(nicTip);
            m_snrMinEdits[i]->setToolTip(nicTip);
        }
        ++row;
    }
}

void QCConfigFileEditorDialog::connectDirtyWatchers()
{
    for (QCheckBox *widget : ui->scrollWidget->findChildren<QCheckBox*>())
        connect(widget, &QCheckBox::toggled, this, &QCConfigFileEditorDialog::markDirty);
    for (QRadioButton *widget : ui->scrollWidget->findChildren<QRadioButton*>())
        connect(widget, &QRadioButton::toggled, this, &QCConfigFileEditorDialog::markDirty);
    for (QLineEdit *widget : ui->scrollWidget->findChildren<QLineEdit*>())
        connect(widget, &QLineEdit::textChanged, this, &QCConfigFileEditorDialog::markDirty);
    for (QComboBox *widget : ui->scrollWidget->findChildren<QComboBox*>())
        connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QCConfigFileEditorDialog::markDirty);
    for (QSpinBox *widget : ui->scrollWidget->findChildren<QSpinBox*>())
        connect(widget, QOverload<int>::of(&QSpinBox::valueChanged), this, &QCConfigFileEditorDialog::markDirty);
    for (QDoubleSpinBox *widget : ui->scrollWidget->findChildren<QDoubleSpinBox*>())
        connect(widget, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QCConfigFileEditorDialog::markDirty);
    for (QDateTimeEdit *widget : ui->scrollWidget->findChildren<QDateTimeEdit*>())
        connect(widget, &QDateTimeEdit::dateTimeChanged, this, &QCConfigFileEditorDialog::markDirty);
}

void QCConfigFileEditorDialog::markDirty()
{
    if (m_updating) return;
    if (!m_dirty)
    {
        m_dirty = true;
        updateTitle();
    }
}

bool QCConfigFileEditorDialog::confirmDiscard()
{
    if (!m_dirty) return true;
    return MyMessageBox::question(this, tr("Unsaved Changes"),
                                  tr("The config has been modified. Discard the changes?"))
           == QMessageBox::Yes;
}

void QCConfigFileEditorDialog::updateTitle()
{
    const QString name = m_filePath.isEmpty() ? tr("Untitled") : QFileInfo(m_filePath).fileName();
    setWindowTitle(QStringLiteral("%1%2 - %3").arg(name, m_dirty ? QStringLiteral("*") : QString(),
                                                   tr("QC Config File Editor")));
}

void QCConfigFileEditorDialog::fillFrom(const GnssCfgFileData &data)
{
    m_updating = true;
    m_clampNotes.clear();

    /**< Strip the dynamic entries a previously loaded file appended, then
     *  re-select; selectOrAppend re-appends unknown values on demand. */
    for (auto it = m_baseComboCounts.constBegin(); it != m_baseComboCounts.constEnd(); ++it)
        while (it.key()->count() > it.value())
            it.key()->removeItem(it.key()->count() - 1);

    selectOrAppend(ui->modeComboBox, data.mode);
    selectOrAppend(ui->rawDataTypeComboBox, data.rawFileDataType);
    selectOrAppend(ui->mpMethodComboBox, (int)data.mpMethod);

    for (int i = 0; i < 7; ++i)
    {
        m_sysChecks[i]->setChecked(data.sysEnabled[i]);
    }
    for (int i = 0; i < 8; ++i)
    {
        /**< Threshold rows follow the letter systems 0..6 (checkbox-linked)
         *  plus the letter-less NavIC row 7, which stays enabled. */
        const bool enabled = (i < 7) ? data.sysEnabled[i] : true;
        m_mpMaxEdits[i]->setEnabled(enabled);
        m_snrMinEdits[i]->setEnabled(enabled);
        m_numSnrEdits[i]->setEnabled(enabled);
        m_mpMaxEdits[i]->setText(joinDoubles(data.mpMax[i]));
        m_snrMinEdits[i]->setText(joinInts(data.snrMin[i]));
        m_numSnrEdits[i]->setText(joinInts(data.numSnrMin[i]));
    }
    ui->banPrnEdit->setText(data.banPrn);
    ui->eleDegSpinBox->setValue((int)data.eleDeg);
    noteClamp(ui->eleDegSpinBox, (int)data.eleDeg, "EleDeg");
    ui->intervalSpinBox->setValue(data.interval);
    noteClamp(ui->intervalSpinBox, data.interval, "Interval");
    ui->snrGroupEdit->setText(joinDoubles(data.snrGroup));
    ui->thresholdSNRChk->setChecked(data.isThresholdSNR);
    ui->thresholdMPChk->setChecked(data.isThresholdMP);
    ui->heightToReflectorSpinBox->setValue(data.heightToReflector);
    noteClamp(ui->heightToReflectorSpinBox, data.heightToReflector, "HeightToReflector");
    ui->mpCarrierSpinBox->setValue(data.mpCarrier);
    noteClamp(ui->mpCarrierSpinBox, data.mpCarrier, "MpCarrier");
    ui->useRateSpinBox->setValue(data.useRate);
    noteClamp(ui->useRateSpinBox, data.useRate, "UseRate");
    ui->cycleJumpRatioSpinBox->setValue((int)data.cycleJumpRatio);
    noteClamp(ui->cycleJumpRatioSpinBox, (int)data.cycleJumpRatio, "CycleJumpRatio");

    ui->outputDataListChk->setChecked(data.isOutputDataList);
    ui->outputQCLogChk->setChecked(data.isOutputQCLog);
    ui->outputQCResBinaryChk->setChecked(data.isOutputQCResBinaryList);
    ui->rbEphDefault->setChecked(data.ephMode == QC_EPH_MODE_AUTO);
    ui->rbEphNone->setChecked(data.ephMode == QC_EPH_MODE_NONE);
    ui->rbEphCustom->setChecked(data.ephMode == QC_EPH_MODE_SPECIFIED);
    if (!ui->rbEphDefault->isChecked() && !ui->rbEphNone->isChecked() && !ui->rbEphCustom->isChecked())
    {
        /**< collectTo falls back to QC_EPH_MODE_AUTO for such values; say so
         *  instead of rewriting the file silently. */
        m_clampNotes << tr("EphMode %1 (from file) has no radio button; saving writes %2 (auto).")
                            .arg(data.ephMode).arg(QC_EPH_MODE_AUTO);
    }
    ui->leEphFilePaths->setText(data.rinexEphFile);

    ui->inputTrueXYZChk->setChecked(data.isInputTrueXYZ);
    /**< XYZ values pass through the engine as doubles: full precision here
     *  keeps an open->save round trip byte-stable ('g',6 would drift). */
    ui->baseTrueXYZEdit->setText(joinXYZ(data.baseTrueXYZ));
    ui->roverTrueXYZEdit->setText(joinXYZ(data.roverTrueXYZ));
    ui->doubleDiffLLIChk->setChecked(data.isDoubleDiffCheckLLI);

    auto applyTimeRow = [](QCheckBox *checkBox, QDateTimeEdit *edit, const QDateTime &time) {
        checkBox->setChecked(time.isValid());
        edit->setEnabled(time.isValid());
        edit->setDateTime(time.isValid() ? time : QDateTime::currentDateTime());
    };
    applyTimeRow(ui->rtcmTimeEnabledCheckBox, ui->rtcmTimeEdit, data.rtcmTime);
    applyTimeRow(ui->startTimeEnabledCheckBox, ui->startDateTimeEdit, data.startTime);
    applyTimeRow(ui->endTimeEnabledCheckBox, ui->endDateTimeEdit, data.endTime);

    m_updating = false;
}

bool QCConfigFileEditorDialog::collectTo(GnssCfgFileData *out, QString *errMsg, QLineEdit **focusEdit)
{
    auto fail = [errMsg, focusEdit](QLineEdit *edit, const QString &message) {
        *errMsg = message;
        if (focusEdit) *focusEdit = edit;
        return false;
    };
    auto parseDoubleEdit = [&](QLineEdit *edit, QVector<double> *values, const QString &fieldName) {
        bool ok = false;
        *values = parseDoubleList(edit->text(), &ok);
        if (!ok) return fail(edit, tr("%1: values must be numbers separated by ','.").arg(fieldName));
        return true;
    };
    auto parseIntEdit = [&](QLineEdit *edit, QVector<int> *values, const QString &fieldName) {
        bool ok = false;
        *values = parseIntList(edit->text(), &ok);
        if (!ok) return fail(edit, tr("%1: values must be integers separated by ','.").arg(fieldName));
        return true;
    };
    auto parseXYZ = [&](QLineEdit *edit, QVector<double> *values, const QString &fieldName) {
        if (edit->text().trimmed().isEmpty())
        {
            *values = {0.0, 0.0, 0.0};
            return true;
        }
        bool ok = false;
        *values = parseDoubleList(edit->text(), &ok);
        if (!ok || values->size() != 3)
            return fail(edit, tr("%1 requires exactly 3 numbers separated by ','.").arg(fieldName));
        return true;
    };

    out->mode = ui->modeComboBox->currentData().toInt();
    out->rawFileDataType = ui->rawDataTypeComboBox->currentData().toInt();
    out->mpMethod = (unsigned int)ui->mpMethodComboBox->currentData().toInt();
    for (int i = 0; i < 7; ++i)
    {
        out->sysEnabled[i] = m_sysChecks[i]->isChecked();
    }
    for (int i = 0; i < 8; ++i)
    {
        if (!parseDoubleEdit(m_mpMaxEdits[i], &out->mpMax[i], systemLabel(i) + tr(" MaxMP")))
            return false;
        if (!parseIntEdit(m_snrMinEdits[i], &out->snrMin[i], systemLabel(i) + tr(" MinSNR")))
            return false;
        if (!parseIntEdit(m_numSnrEdits[i], &out->numSnrMin[i], systemLabel(i) + tr(" MinSNR Sat Num")))
            return false;
    }
    out->banPrn = ui->banPrnEdit->text().trimmed();
    out->eleDeg = (unsigned int)ui->eleDegSpinBox->value();
    out->interval = ui->intervalSpinBox->value();
    if (!parseDoubleEdit(ui->snrGroupEdit, &out->snrGroup, tr("SNR Group"))) return false;
    out->isThresholdSNR = ui->thresholdSNRChk->isChecked();
    out->isThresholdMP = ui->thresholdMPChk->isChecked();
    out->heightToReflector = ui->heightToReflectorSpinBox->value();
    out->mpCarrier = ui->mpCarrierSpinBox->value();
    out->useRate = ui->useRateSpinBox->value();
    out->cycleJumpRatio = (unsigned int)ui->cycleJumpRatioSpinBox->value();

    out->isOutputDataList = ui->outputDataListChk->isChecked();
    out->isOutputQCLog = ui->outputQCLogChk->isChecked();
    out->isOutputQCResBinaryList = ui->outputQCResBinaryChk->isChecked();
    if (ui->rbEphNone->isChecked())
        out->ephMode = QC_EPH_MODE_NONE;
    else if (ui->rbEphCustom->isChecked())
        out->ephMode = QC_EPH_MODE_SPECIFIED;
    else
        out->ephMode = QC_EPH_MODE_AUTO;
    out->rinexEphFile = ui->leEphFilePaths->text().trimmed();

    out->isInputTrueXYZ = ui->inputTrueXYZChk->isChecked();
    if (!parseXYZ(ui->baseTrueXYZEdit, &out->baseTrueXYZ, tr("Base True XYZ"))) return false;
    if (!parseXYZ(ui->roverTrueXYZEdit, &out->roverTrueXYZ, tr("Rover True XYZ"))) return false;
    out->isDoubleDiffCheckLLI = ui->doubleDiffLLIChk->isChecked();

    out->rtcmTime = ui->rtcmTimeEnabledCheckBox->isChecked() ? ui->rtcmTimeEdit->dateTime() : QDateTime();
    out->startTime = ui->startTimeEnabledCheckBox->isChecked() ? ui->startDateTimeEdit->dateTime() : QDateTime();
    out->endTime = ui->endTimeEnabledCheckBox->isChecked() ? ui->endDateTimeEdit->dateTime() : QDateTime();
    return true;
}

/**< Selects @p value in @p combo, appending a "(from file)" entry for values
 *  outside the standard list: unknown values stay stable across a load/save
 *  cycle instead of being silently rewritten to the first entry. */
void QCConfigFileEditorDialog::selectOrAppend(QComboBox *combo, int value)
{
    int index = combo->findData(value);
    if (index < 0)
    {
        combo->addItem(tr("%1: (from file)").arg(value), value);
        index = combo->count() - 1;
    }
    combo->setCurrentIndex(index);
}

/**< Registers a note when @p raw falls outside the spin box range and would
 *  be silently clamped by setValue(); notes surface after loading a file. */
void QCConfigFileEditorDialog::noteClamp(QSpinBox *box, int raw, const char *field)
{
    const int clamped = qBound(box->minimum(), raw, box->maximum());
    if (clamped != raw)
        m_clampNotes << tr("%1 was adjusted to %2 (supported range %3..%4).")
                            .arg(QLatin1String(field)).arg(clamped)
                            .arg(box->minimum()).arg(box->maximum());
}

/**< Double overload additionally detects silent decimals rounding, which the
 *  already-executed setValue() may have applied to an in-range value. */
void QCConfigFileEditorDialog::noteClamp(QDoubleSpinBox *box, double raw, const char *field)
{
    const double clamped = qBound(box->minimum(), raw, box->maximum());
    if (!qFuzzyCompare(clamped, raw))
    {
        m_clampNotes << tr("%1 was adjusted to %2 (supported range %3..%4).")
                            .arg(QLatin1String(field)).arg(clamped)
                            .arg(box->minimum()).arg(box->maximum());
        return;
    }
    if (!qFuzzyCompare(box->value(), raw))
        m_clampNotes << tr("%1 was rounded to %2 (%3 decimals).")
                            .arg(QLatin1String(field)).arg(box->value()).arg(box->decimals());
}

void QCConfigFileEditorDialog::onNew()
{
    if (!confirmDiscard()) return;
    fillFrom(defaultGnssCfgFileData());
    m_filePath.clear();
    ui->filePathEdit->clear();
    m_dirty = false;
    updateTitle();
}

void QCConfigFileEditorDialog::onOpen()
{
    if (!confirmDiscard()) return;
    const QString path = QFileDialog::getOpenFileName(this, tr("Open QC Config File"), QString(),
                                                      tr("Config Files (*.txt);;All Files (*)"));
    if (path.isEmpty()) return;
    GnssCfgFileData data;
    QString err;
    QString dropped;
    QString sysNote;
    if (!loadGnssCfgFile(path, &data, &err, &dropped, &sysNote))
    {
        MyMessageBox::critical(this, tr("Error"), err);
        return;
    }
    fillFrom(data);
    /**< fillFrom clears m_clampNotes, so the dropped-keys note joins after it. */
    if (!dropped.isEmpty()) m_clampNotes << dropped;
    if (!sysNote.isEmpty()) m_clampNotes << sysNote;
    m_filePath = path;
    ui->filePathEdit->setText(path);
    m_dirty = false;
    updateTitle();
    if (!m_clampNotes.isEmpty())
    {
        MyMessageBox::warning(this, tr("Values Adjusted"),
                              tr("Some values are outside the supported ranges and were adjusted:\n%1")
                                  .arg(m_clampNotes.join(QLatin1Char('\n'))));
    }
}

bool QCConfigFileEditorDialog::saveTo(const QString &path)
{
    GnssCfgFileData data;
    QString err;
    QLineEdit *focusEdit = nullptr;
    if (!collectTo(&data, &err, &focusEdit))
    {
        MyMessageBox::critical(this, tr("Error"), err);
        if (focusEdit)
        {
            focusEdit->setFocus();
            focusEdit->selectAll();
        }
        return false;
    }
    QString warn;
    if (!validateGnssCfgFileData(data, &err, &warn))
    {
        MyMessageBox::critical(this, tr("Error"), err);
        return false;
    }
    if (!warn.isEmpty() &&
        MyMessageBox::question(this, tr("Warning"), warn + QLatin1Char('\n') + tr("Save anyway?"),
                               QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) != QMessageBox::Yes)
    {
        return false;
    }
    if (!saveGnssCfgFile(path, data, &err))
    {
        MyMessageBox::critical(this, tr("Error"), err);
        return false;
    }
    m_filePath = path;
    ui->filePathEdit->setText(path);
    m_dirty = false;
    updateTitle();
    return true;
}

void QCConfigFileEditorDialog::onSave()
{
    if (m_filePath.isEmpty())
    {
        onSaveAs();
        return;
    }
    saveTo(m_filePath);
}

void QCConfigFileEditorDialog::onSaveAs()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Save QC Config File As"), m_filePath,
                                                tr("Config Files (*.txt);;All Files (*)"));
    if (path.isEmpty()) return;
    if (!path.endsWith(QStringLiteral(".txt"), Qt::CaseInsensitive)) path += QStringLiteral(".txt");
    saveTo(path);
}

void QCConfigFileEditorDialog::closeEvent(QCloseEvent *event)
{
    if (confirmDiscard())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void QCConfigFileEditorDialog::reject()
{
    if (confirmDiscard())
    {
        MyDialog::reject();
    }
}
