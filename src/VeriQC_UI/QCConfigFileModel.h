/**********************************************************************//**
 * 		VeriQC
 *
 * 		UI Module
 *-
 * @file QCConfigFileModel.h
 * @author CHC
 * @date 2026-09-02
 * @brief UI-side model + file IO for the engine config files
 *
 *        Models the config keys the editors expose (see
 *        src/VeriQC/doc/GNSSConfig.txt / RinexConfig.txt for the full engine
 *        format). Loading goes through the engine Read API (ReadGNSSCfg /
 *        ReadRinexCfg) so the form always shows exactly what the engine would
 *        consume on the command line (-c); saving serialises the model back
 *        into the same text format. The engine has no "write" export, so file
 *        generation lives entirely on the UI side.
 *
 *        Frequency selection and file paths are deliberately NOT modelled:
 *        the product UI has no frequency selection (AllFreq is always written
 *        as 1 = all frequencies) and the console overrides every path field
 *        per run (-o / input dir for OutDir, one per file for the inputs).
 *        The prescan still gates those keys when loading foreign files.
 *
 *        All per-system threshold arrays are indexed 0..7: rows 0..5 follow
 *        the engine order GPS, SBAS, GLONASS, Galileo, QZSS, BDS (engine
 *        fields G*, S*, R*, E*, J*, C*; file keys *MpMAX / *SnrMIN /
 *        Num*SnrMIN), row 6 is XW1 (engine fields L*) and row 7 is NavIC
 *        (NIC* keys kept for round-trip fidelity only — the QC engine
 *        parses but never consumes them).
 *        The Sys line instead carries the engine's Str2Sys letters
 *        G,S,R,E,J,C,L, where 'L' sets the 0x80 system bit — the engine no
 *        longer accepts 'I' since the XW1 update. The editor is WYSIWYG with
 *        the engine.
**************************************************************************/
#ifndef QCCONFIGFILEMODEL_H
#define QCCONFIGFILEMODEL_H

#include <QDateTime>
#include <QString>
#include <QVector>
#include "VeriQC.h"

struct GnssCfgFileData
{
    int mode = -1;                        /**< Mode: -1 none, 0 zero baseline, 1 diff sat/epoch, 2 short baseline, 3 triple diff */
    int rawFileDataType = -1;             /**< RawFileDataType (GNSS_STRFMT_xx); -1 = auto detect */
    bool sysEnabled[7] = {};              /**< Sys letters G,S,R,E,J,C,L */
    QString banPrn;                       /**< BanPRN raw value, pass-through (engine accepts any separators) */
    unsigned int eleDeg = 10;             /**< EleDeg elevation mask [deg] */
    double interval = 1.0;                /**< Interval [s], 0 = from file */
    QVector<double> snrGroup;             /**< SNRGroup elevation groups [deg], at most QC_MAXELEVINTERV items */
    bool isThresholdSNR = true;           /**< IsThresholdSNR */
    bool isThresholdMP = true;            /**< IsThresholdMP */
    double heightToReflector = 0.0;       /**< HeightToReflector [m] */
    QDateTime rtcmTime;                   /**< RTCMTime; invalid => line written commented out */
    QDateTime startTime;                  /**< StartTime; invalid => line written commented out */
    QDateTime endTime;                    /**< EndTime; invalid => line written commented out */
    QVector<double> baseTrueXYZ;          /**< BaseTrueXYZ, 3 values */
    QVector<double> roverTrueXYZ;         /**< RoverTrueXYZ, 3 values */
    bool isInputTrueXYZ = false;          /**< IsInputTrueXYZ */
    bool isOutputDataList = false;        /**< IsOutputDataList */
    bool isOutputQCLog = false;           /**< IsOutputQCLog */
    bool isOutputQCResBinaryList = false; /**< IsOutputQCResBinaryList */
    int ephMode = 0;                      /**< EphMode: QC_EPH_MODE_AUTO(0)/NONE(1)/SPECIFIED(2) */
    QString rinexEphFile;                 /**< RinexEphFile */
    unsigned int mpMethod = 0;            /**< MPMethod: 0 fixed 50-epoch, 1 filter */
    QVector<double> mpMax[8];             /**< *MpMAX per threshold row [m]; rows 0..5 GPS..BDS, 6 XW1, 7 NavIC (no letter) */
    QVector<int> snrMin[8];               /**< *SnrMIN per threshold row [dBHz] */
    QVector<int> numSnrMin[8];            /**< Num*SnrMIN per threshold row */
    double mpCarrier = 0.005;             /**< MpCarrier [m] */
    double useRate = 95.0;                /**< UseRate [%] */
    unsigned int cycleJumpRatio = 400;    /**< CycleJumpRatio */
    bool isDoubleDiffCheckLLI = false;    /**< IsDoubleDiffCheckLLI */
};

struct RinexCfgFileData
{
    int dataType = 11;                    /**< DataType (GNSS_STRFMT_xx); default RINEX */
    int outputType = 0;                   /**< OutputType (DATA_OUT_TYPE_xx); 0 = RINEX */
    int rinexVersion = 304;               /**< RinexVer x100 (3.04 -> 304) */
    double interval = 0.0;                /**< Interval [s], 0 = from raw file */
    bool sysEnabled[7] = {};              /**< Sys string letters G,S,R,E,J,C,I (Rinex converter letter set; 'I' = NavIC) */
    unsigned int split = 0;               /**< Split: 0 none, 1 station moving, 2 time interval, 3 time period */
    unsigned int splitTimeInt = 3600;     /**< SplitTint [s] */
    QDateTime splitStart;                 /**< SplitStart; invalid => line written commented out */
    QDateTime splitEnd;                   /**< SplitEnd; invalid => line written commented out */
    QDateTime rtcmTime;                   /**< RTCMTime; invalid => line written commented out */
    bool cheakSeq = false;                /**< CheakSeq (engine key spelling) */
    unsigned int leapSecond = 0;          /**< LeapSecond: 0 from eph */
    bool pause = false;                   /**< Pause */
};

namespace QCConfigFileModel {

/**< Pristine "new file" state: doc-sample defaults for the fields that
 *  InitGNSSCfg leaves uninitialized. */
GnssCfgFileData defaultGnssCfgFileData();
/**< Pristine "new file" state for the Rinex convert config. */
RinexCfgFileData defaultRinexCfgFileData();

/**< Loads a GNSS QC config file through the engine parser; fails when the
 *  prescan gate or the section marker rejects it. When @p droppedKeysNote is
 *  given it receives a message listing the keys the editor no longer writes
 *  (frequency options / file paths) when the file carries them. When
 *  @p sysNote is given it receives the legacy 'I' Sys-letter migration hint
 *  (the QC parser Str2Sys no longer accepts 'I'; saving writes 'L'). */
bool loadGnssCfgFile(const QString &path, GnssCfgFileData *out, QString *errMsg, QString *droppedKeysNote = nullptr, QString *sysNote = nullptr);
/**< Loads a Rinex convert config file through the engine parser; same
 *  dropped-keys contract as loadGnssCfgFile (adds ObsType). */
bool loadRinexCfgFile(const QString &path, RinexCfgFileData *out, QString *errMsg, QString *droppedKeysNote = nullptr);
/**< Validates and serializes @p data; runs the prescan on the generated
 *  bytes before anything touches the disk. */
bool saveGnssCfgFile(const QString &path, const GnssCfgFileData &data, QString *errMsg);
/**< Validates and serializes a Rinex convert config. */
bool saveRinexCfgFile(const QString &path, const RinexCfgFileData &data, QString *errMsg);

/**< @p warnMsg collects non-fatal hints (the save proceeds); @p errMsg blocks it. */
bool validateGnssCfgFileData(const GnssCfgFileData &data, QString *errMsg, QString *warnMsg);
/**< Same contract for the Rinex convert config (no warnings today). */
bool validateRinexCfgFileData(const RinexCfgFileData &data, QString *errMsg, QString *warnMsg);

/**< x100 integer form ("304") as "3.04" for display and file output. */
QString rinexVersionText(int rinexVersion);
int systemFreqCount(int systemIndex);     /**< engine QC_*_FRE_NUM for the given threshold-row index (6=XW1, 7=NavIC) */
QString systemFreqNames(int systemIndex); /**< comma-separated freq names for tooltips / column captions */
QString systemLabel(int systemIndex);     /**< GPS / SBAS / GLONASS / ... / XW1 / NavIC */
QChar systemLetter(int systemIndex);      /**< G S R E J C L (GNSS side; undefined for row 7 NavIC) */

QString joinDoubles(const QVector<double> &values); /**< comma list, used by the editors */
QString joinXYZ(const QVector<double> &values);     /**< comma list at full double precision (TrueXYZ), used by the editors */
QString joinInts(const QVector<int> &values);       /**< comma list, used by the editors */

} // namespace QCConfigFileModel

#endif // QCCONFIGFILEMODEL_H
