/**********************************************************************//**
 * 		VeriQC
 *
 * 		UI Module
 *-
 * @file QCConfigFileEditorDialog.h
 * @author CHC
 * @date 2026-09-02
 * @brief Modal editor for the engine GNSS QC config file
 *        (doc/GNSSConfig.txt format; see QCConfigFileModel.h).
 *
 *        New / Open / Save / Save As with a dirty guard on every exit path;
 *        validation failures report the offending field and focus it.
**************************************************************************/
#ifndef QCCONFIGFILEEDITORDIALOG_H
#define QCCONFIGFILEEDITORDIALOG_H

#include "Components/MyDialog.h"
#include "QCConfigFileModel.h"

#include <QHash>
#include <QStringList>

class QCheckBox;
class QCloseEvent;
class QComboBox;
class QDoubleSpinBox;
class QEvent;
class QLineEdit;
class QSpinBox;

namespace Ui {
class QCConfigFileEditorDialog;
}

class QCConfigFileEditorDialog : public MyDialog
{
    Q_OBJECT

public:
    explicit QCConfigFileEditorDialog(QWidget *parent = nullptr);
    ~QCConfigFileEditorDialog();

protected:
    /**< Warn-and-stay when the user closes the dialog with unsaved edits. */
    void closeEvent(QCloseEvent *event) override;
    /**< Same discard guard as closeEvent for the Esc / reject path. */
    void reject() override;
    /**< Routes wheel events from form children to the scroll area so scrolling
     *  never changes combo / spin box values. */
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    /**< Discards the current state (after confirmation) and reloads defaults. */
    void onNew();
    /**< Loads a config file after a discard confirmation. */
    void onOpen();
    /**< Collects, validates and writes to the current path (Save As first). */
    void onSave();
    /**< Collects, validates and writes to a user-chosen path. */
    void onSaveAs();
    /**< Marks the form dirty; connected to every editor widget. */
    void markDirty();

private:
    /**< Builds the eight per-system threshold rows (7 selectable systems plus the letter-less NavIC row) into the grid. */
    void buildThresholdRows();
    /**< Wires every editor widget inside the scroll area to markDirty(). */
    void connectDirtyWatchers();
    /**< True when the form is clean or the user confirmed discarding edits. */
    bool confirmDiscard();
    /**< Window title with dirty marker and current file name. */
    void updateTitle();
    /**< Pushes @p data into all widgets and collects clamp notes. */
    void fillFrom(const GnssCfgFileData &data);
    /**< Collects all widgets into @p out; on error fills @p errMsg and points
     *  @p focusEdit at the offending field. */
    bool collectTo(GnssCfgFileData *out, QString *errMsg, QLineEdit **focusEdit);
    /**< Validates, warns and writes the collected data to @p path. */
    bool saveTo(const QString &path);
    /**< Selects @p value in @p combo, appending a "(from file)" entry for
     *  values outside the standard list so unknown values survive a
     *  load/save cycle instead of being rewritten to the first entry. */
    void selectOrAppend(QComboBox *combo, int value);
    /**< Records a note when @p raw falls outside the spin box range and
     *  setValue() would silently clamp it (surfaces after loading a file). */
    void noteClamp(QSpinBox *box, int raw, const char *field);
    /**< Same as the integer overload for double spin boxes. */
    void noteClamp(QDoubleSpinBox *box, double raw, const char *field);

    Ui::QCConfigFileEditorDialog *ui;
    QCheckBox *m_sysChecks[7] = {};       /**< 7 Sys letters; threshold row 7 (NavIC) has no checkbox */
    QLineEdit *m_mpMaxEdits[8] = {};      /**< threshold rows 0..5 GPS..BDS, 6 XW1, 7 NavIC */
    QLineEdit *m_snrMinEdits[8] = {};
    QLineEdit *m_numSnrEdits[8] = {};
    QString m_filePath;
    bool m_dirty = false;
    bool m_updating = false;
    QHash<QComboBox *, int> m_baseComboCounts; /**< standard item counts for fillFrom truncation */
    QStringList m_clampNotes;                  /**< fields clamped by the last fillFrom */
};

#endif // QCCONFIGFILEEDITORDIALOG_H
