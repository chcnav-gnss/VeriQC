/**********************************************************************//**
 * 		VeriQC
 *
 * 		UI Module
 *-
 * @file RinexConfigFileEditorDialog.cpp
 * @author CHC
 * @date 2026-09-02
 * @brief Modal editor for the engine Rinex convert config file
**************************************************************************/
#include "RinexConfigFileEditorDialog.h"
#include "ui_RinexConfigFileEditorDialog.h"

#include "Components/MyMessageBox.h"

#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QLineEdit>
#include <QPushButton>
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

} // namespace

RinexConfigFileEditorDialog::RinexConfigFileEditorDialog(QWidget *parent)
    : MyDialog(parent)
    , ui(new Ui::RinexConfigFileEditorDialog)
{
    ui->setupUi(this);
    setMinimumWidth(560); /**< Logical pixels under High-DPI PassThrough, same contract as QCConfigDialog. */

    ui->dataTypeComboBox->clear();
    ui->dataTypeComboBox->addItem(tr("-1: Auto detect"), -1);
    ui->dataTypeComboBox->addItem(tr("0: RTCM2"), 0);
    ui->dataTypeComboBox->addItem(tr("1: RTCM3"), 1);
    ui->dataTypeComboBox->addItem(tr("11: RINEX"), 11);
    ui->dataTypeComboBox->addItem(tr("37: HRCX"), 37);
    ui->dataTypeComboBox->addItem(tr("38: HRC"), 38);
    ui->dataTypeComboBox->addItem(tr("51: RawMsg ASCII"), 51);
    ui->dataTypeComboBox->addItem(tr("52: RawMsg BIN"), 52);
    ui->dataTypeComboBox->addItem(tr("53: BaseObs ASCII"), 53);
    ui->dataTypeComboBox->addItem(tr("54: BaseObs BIN"), 54);
    ui->dataTypeComboBox->addItem(tr("55: UNRAWMSGB"), 55);
    ui->dataTypeComboBox->addItem(tr("56: CHCRAWMSGB"), 56);
    ui->dataTypeComboBox->addItem(tr("101: KMD RawMsg ASCII"), 101);
    ui->dataTypeComboBox->addItem(tr("102: KMD RawMsg BIN"), 102);
    ui->dataTypeComboBox->addItem(tr("103: RTKLOGA"), 103);
    ui->dataTypeComboBox->addItem(tr("104: RTKLOGB"), 104);
    ui->dataTypeComboBox->addItem(tr("105: CLAS"), 105);

    ui->outputTypeComboBox->clear();
    ui->outputTypeComboBox->addItem(tr("0: RINEX"), 0);

    ui->rinexVerComboBox->clear();
    for (int version : {211, 301, 302, 303, 304, 400})
    {
        ui->rinexVerComboBox->addItem(rinexVersionText(version), version);
    }

    ui->splitComboBox->clear();
    ui->splitComboBox->addItem(tr("0: No split"), 0);
    ui->splitComboBox->addItem(tr("1: Station moving"), 1);
    ui->splitComboBox->addItem(tr("2: Time interval"), 2);
    ui->splitComboBox->addItem(tr("3: Time period"), 3);

    m_sysChecks[0] = ui->sysGPSChk;
    m_sysChecks[1] = ui->sysSBASChk;
    m_sysChecks[2] = ui->sysGLOChk;
    m_sysChecks[3] = ui->sysGALChk;
    m_sysChecks[4] = ui->sysQZSChk;
    m_sysChecks[5] = ui->sysBDSChk;
    m_sysChecks[6] = ui->sysNICChk;

    connectSectionToggle(ui->basicToolButton, ui->basicWidget);
    connectSectionToggle(ui->splitToolButton, ui->splitWidget);

    /**< Expert review 2026-09: splitting is no longer supported by the
     *  product, so the split controls stay out of sight. They (and the model
     *  behind them) keep working verbatim: an opened file still round-trips
     *  the Split/SplitTint/SplitStart/SplitEnd keys, a new file writes the
     *  model defaults (Split 0, start/end lines commented out). */
    ui->splitLabel->hide();
    ui->splitComboBox->hide();
    ui->splitTintLabel->hide();
    ui->splitTintSpinBox->hide();
    ui->splitStartEnabledChk->hide();
    ui->splitStartDateTimeEdit->hide();
    ui->splitEndEnabledChk->hide();
    ui->splitEndDateTimeEdit->hide();

    connect(ui->splitComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        const int split = index >= 0 ? ui->splitComboBox->itemData(index).toInt() : 0;
        ui->splitTintSpinBox->setEnabled(split == 2);
    });
    /**< Keep start < end interactively (same contract as RinexConvertSettingDialog);
     *  the guard keeps fillFrom from clamping a file whose times deliberately
     *  disagree. */
    connect(ui->splitStartDateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, [this](const QDateTime &dt) {
        if (m_updating) return;
        if (ui->splitEndDateTimeEdit->dateTime() < dt)
            ui->splitEndDateTimeEdit->setDateTime(dt.addSecs(1));
    });
    connect(ui->splitEndDateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, [this](const QDateTime &dt) {
        if (m_updating) return;
        if (ui->splitStartDateTimeEdit->dateTime() > dt)
            ui->splitStartDateTimeEdit->setDateTime(dt.addSecs(-1));
    });
    /**< Checked: the time line is written to the file; unchecked: it is
     *  written commented out (mirrors the file round-trip). */
    connect(ui->splitStartEnabledChk, &QCheckBox::toggled, ui->splitStartDateTimeEdit, &QWidget::setEnabled);
    connect(ui->splitEndEnabledChk, &QCheckBox::toggled, ui->splitEndDateTimeEdit, &QWidget::setEnabled);
    connect(ui->rtcmTimeEnabledCheckBox, &QCheckBox::toggled, ui->rtcmTimeEdit, &QDateTimeEdit::setEnabled);

    connect(ui->btnNew, &QPushButton::clicked, this, &RinexConfigFileEditorDialog::onNew);
    connect(ui->btnOpen, &QPushButton::clicked, this, &RinexConfigFileEditorDialog::onOpen);
    connect(ui->btnSave, &QPushButton::clicked, this, &RinexConfigFileEditorDialog::onSave);
    connect(ui->btnSaveAs, &QPushButton::clicked, this, &RinexConfigFileEditorDialog::onSaveAs);
    connect(ui->btnClose, &QPushButton::clicked, this, &RinexConfigFileEditorDialog::reject);

    connectDirtyWatchers();

    /**< Remember the standard item counts so fillFrom can strip dynamic
     *  entries appended for a previously loaded file. */
    m_baseComboCounts.insert(ui->dataTypeComboBox, ui->dataTypeComboBox->count());
    m_baseComboCounts.insert(ui->outputTypeComboBox, ui->outputTypeComboBox->count());
    m_baseComboCounts.insert(ui->splitComboBox, ui->splitComboBox->count());
    m_baseComboCounts.insert(ui->rinexVerComboBox, ui->rinexVerComboBox->count());

    fillFrom(defaultRinexCfgFileData());
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

RinexConfigFileEditorDialog::~RinexConfigFileEditorDialog()
{
    delete ui;
}

bool RinexConfigFileEditorDialog::eventFilter(QObject *watched, QEvent *event)
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

void RinexConfigFileEditorDialog::connectDirtyWatchers()
{
    for (QCheckBox *widget : ui->scrollWidget->findChildren<QCheckBox*>())
        connect(widget, &QCheckBox::toggled, this, &RinexConfigFileEditorDialog::markDirty);
    for (QLineEdit *widget : ui->scrollWidget->findChildren<QLineEdit*>())
        connect(widget, &QLineEdit::textChanged, this, &RinexConfigFileEditorDialog::markDirty);
    for (QComboBox *widget : ui->scrollWidget->findChildren<QComboBox*>())
        connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &RinexConfigFileEditorDialog::markDirty);
    for (QSpinBox *widget : ui->scrollWidget->findChildren<QSpinBox*>())
        connect(widget, QOverload<int>::of(&QSpinBox::valueChanged), this, &RinexConfigFileEditorDialog::markDirty);
    for (QDoubleSpinBox *widget : ui->scrollWidget->findChildren<QDoubleSpinBox*>())
        connect(widget, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &RinexConfigFileEditorDialog::markDirty);
    for (QDateTimeEdit *widget : ui->scrollWidget->findChildren<QDateTimeEdit*>())
        connect(widget, &QDateTimeEdit::dateTimeChanged, this, &RinexConfigFileEditorDialog::markDirty);
}

void RinexConfigFileEditorDialog::markDirty()
{
    if (m_updating) return;
    if (!m_dirty)
    {
        m_dirty = true;
        updateTitle();
    }
}

bool RinexConfigFileEditorDialog::confirmDiscard()
{
    if (!m_dirty) return true;
    return MyMessageBox::question(this, tr("Unsaved Changes"),
                                  tr("The config has been modified. Discard the changes?"))
           == QMessageBox::Yes;
}

void RinexConfigFileEditorDialog::updateTitle()
{
    const QString name = m_filePath.isEmpty() ? tr("Untitled") : QFileInfo(m_filePath).fileName();
    setWindowTitle(QStringLiteral("%1%2 - %3").arg(name, m_dirty ? QStringLiteral("*") : QString(),
                                                   tr("Rinex Convert Config File Editor")));
}

void RinexConfigFileEditorDialog::setRinexVersion(int rinexVersion)
{
    int index = ui->rinexVerComboBox->findData(rinexVersion);
    if (index < 0)
    {
        ui->rinexVerComboBox->addItem(rinexVersionText(rinexVersion), rinexVersion);
        index = ui->rinexVerComboBox->count() - 1;
    }
    ui->rinexVerComboBox->setCurrentIndex(index);
}

void RinexConfigFileEditorDialog::fillFrom(const RinexCfgFileData &data)
{
    m_updating = true;
    m_clampNotes.clear();

    /**< Strip the dynamic entries a previously loaded file appended, then
     *  re-select; selectOrAppend / setRinexVersion re-append on demand. */
    for (auto it = m_baseComboCounts.constBegin(); it != m_baseComboCounts.constEnd(); ++it)
        while (it.key()->count() > it.value())
            it.key()->removeItem(it.key()->count() - 1);

    selectOrAppend(ui->dataTypeComboBox, data.dataType);
    selectOrAppend(ui->outputTypeComboBox, data.outputType);
    selectOrAppend(ui->splitComboBox, (int)data.split);
    setRinexVersion(data.rinexVersion);
    ui->intervalSpinBox->setValue(data.interval);
    noteClamp(ui->intervalSpinBox, data.interval, "Interval");

    for (int i = 0; i < 7; ++i)
    {
        m_sysChecks[i]->setChecked(data.sysEnabled[i]);
    }
    ui->leapSecondSpinBox->setValue((int)data.leapSecond);
    noteClamp(ui->leapSecondSpinBox, (int)data.leapSecond, "LeapSecond");
    ui->cheakSeqChk->setChecked(data.cheakSeq);
    ui->pauseChk->setChecked(data.pause);

    ui->splitTintSpinBox->setValue((int)data.splitTimeInt);
    noteClamp(ui->splitTintSpinBox, (int)data.splitTimeInt, "SplitTint");
    ui->splitTintSpinBox->setEnabled(data.split == 2);
    /**< The checkboxes carry the line-active state: an invalid time from the
     *  file means the line was commented out and is written commented out. */
    auto applyTimeRow = [](QCheckBox *checkBox, QDateTimeEdit *edit, const QDateTime &time) {
        checkBox->setChecked(time.isValid());
        edit->setEnabled(time.isValid());
        edit->setDateTime(time.isValid() ? time : QDateTime::currentDateTime());
    };
    applyTimeRow(ui->splitStartEnabledChk, ui->splitStartDateTimeEdit, data.splitStart);
    applyTimeRow(ui->splitEndEnabledChk, ui->splitEndDateTimeEdit, data.splitEnd);
    applyTimeRow(ui->rtcmTimeEnabledCheckBox, ui->rtcmTimeEdit, data.rtcmTime);

    m_updating = false;
}

bool RinexConfigFileEditorDialog::collectTo(RinexCfgFileData *out, QString *errMsg, QLineEdit **focusEdit)
{
    out->dataType = ui->dataTypeComboBox->currentData().toInt();
    out->outputType = ui->outputTypeComboBox->currentData().toInt();
    out->rinexVersion = ui->rinexVerComboBox->currentData().toInt();
    out->interval = ui->intervalSpinBox->value();
    for (int i = 0; i < 7; ++i)
    {
        out->sysEnabled[i] = m_sysChecks[i]->isChecked();
    }
    out->split = (unsigned int)ui->splitComboBox->currentData().toInt();
    out->splitTimeInt = (unsigned int)ui->splitTintSpinBox->value();
    out->splitStart = ui->splitStartEnabledChk->isChecked() ? ui->splitStartDateTimeEdit->dateTime() : QDateTime();
    out->splitEnd = ui->splitEndEnabledChk->isChecked() ? ui->splitEndDateTimeEdit->dateTime() : QDateTime();
    out->rtcmTime = ui->rtcmTimeEnabledCheckBox->isChecked() ? ui->rtcmTimeEdit->dateTime() : QDateTime();
    out->cheakSeq = ui->cheakSeqChk->isChecked();
    out->leapSecond = (unsigned int)ui->leapSecondSpinBox->value();
    out->pause = ui->pauseChk->isChecked();
    Q_UNUSED(errMsg);
    Q_UNUSED(focusEdit);
    return true;
}

/**< Selects @p value in @p combo, appending a "(from file)" entry for values
 *  outside the standard list: unknown values stay stable across a load/save
 *  cycle instead of being silently rewritten to the first entry. */
void RinexConfigFileEditorDialog::selectOrAppend(QComboBox *combo, int value)
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
void RinexConfigFileEditorDialog::noteClamp(QSpinBox *box, int raw, const char *field)
{
    const int clamped = qBound(box->minimum(), raw, box->maximum());
    if (clamped != raw)
        m_clampNotes << tr("%1 was adjusted to %2 (supported range %3..%4).")
                            .arg(QLatin1String(field)).arg(clamped)
                            .arg(box->minimum()).arg(box->maximum());
}

/**< Registers a note when @p raw falls outside the spin box range, or gets
 *  rounded to the box decimals by the already-executed setValue(); notes
 *  surface after loading a file. */
void RinexConfigFileEditorDialog::noteClamp(QDoubleSpinBox *box, double raw, const char *field)
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

void RinexConfigFileEditorDialog::onNew()
{
    if (!confirmDiscard()) return;
    fillFrom(defaultRinexCfgFileData());
    m_filePath.clear();
    ui->filePathEdit->clear();
    m_dirty = false;
    updateTitle();
}

void RinexConfigFileEditorDialog::onOpen()
{
    if (!confirmDiscard()) return;
    const QString path = QFileDialog::getOpenFileName(this, tr("Open Rinex Convert Config File"), QString(),
                                                      tr("Config Files (*.txt);;All Files (*)"));
    if (path.isEmpty()) return;
    RinexCfgFileData data;
    QString err;
    QString dropped;
    if (!loadRinexCfgFile(path, &data, &err, &dropped))
    {
        MyMessageBox::critical(this, tr("Error"), err);
        return;
    }
    fillFrom(data);
    /**< fillFrom clears m_clampNotes, so the dropped-keys note joins after it. */
    if (!dropped.isEmpty()) m_clampNotes << dropped;
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

bool RinexConfigFileEditorDialog::saveTo(const QString &path)
{
    RinexCfgFileData data;
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
    if (!validateRinexCfgFileData(data, &err, &warn))
    {
        MyMessageBox::critical(this, tr("Error"), err);
        return false;
    }
    if (!saveRinexCfgFile(path, data, &err))
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

void RinexConfigFileEditorDialog::onSave()
{
    if (m_filePath.isEmpty())
    {
        onSaveAs();
        return;
    }
    saveTo(m_filePath);
}

void RinexConfigFileEditorDialog::onSaveAs()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Save Rinex Convert Config File As"), m_filePath,
                                                tr("Config Files (*.txt);;All Files (*)"));
    if (path.isEmpty()) return;
    if (!path.endsWith(QStringLiteral(".txt"), Qt::CaseInsensitive)) path += QStringLiteral(".txt");
    saveTo(path);
}

void RinexConfigFileEditorDialog::closeEvent(QCloseEvent *event)
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

void RinexConfigFileEditorDialog::reject()
{
    if (confirmDiscard())
    {
        MyDialog::reject();
    }
}
