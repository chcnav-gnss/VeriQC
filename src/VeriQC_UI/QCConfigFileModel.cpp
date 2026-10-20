/**********************************************************************//**
 * 		VeriQC
 *
 * 		UI Module
 *-
 * @file QCConfigFileModel.cpp
 * @author CHC
 * @date 2026-09-02
 * @brief UI-side model + file IO for the engine config files
 *
 *        Engine parsing facts this code relies on (all verified in
 *        src/VeriQC/src/Interface/QCCommon.c):
 *        - ReadGNSSCfg / ReadRinexCfg read fgets(line[255]) chunks, split at
 *          the first '=', truncate the value at '#' (strtok) and memcpy
 *          string values WITHOUT clamping, so opening a hostile file can
 *          overflow the fixed char[] fields — loadConfigText() + prescan
 *          keeps such files away from the engine.
 *        - The GNSS parser maps Sys letters G,S,R,E,J,C,L with 'L' going to
 *          QC_SYS_XW1 (the XW1/LEO system bit), and an empty Sys becomes
 *          255 (all).
 *        - SNRGroup parsing loses the last character of the last item
 *          ("30" -> {3}); the editor shows what the engine sees.
 *        - OutDir always gets a trailing '\' appended by the parser.
 *        - QC freq option lines are only decoded when AllFreq != 1 and the
 *          decoder is whitespace sensitive. The editors no longer model them
 *          (AllFreq is written as 1 and the lines are dropped on save), but
 *          the prescan still gates them for the engine, as it does for the
 *          path keys the console overrides anyway.
**************************************************************************/
#include "QCConfigFileModel.h"
#include "PathUtils.h"

#include <QFile>
#include <QHash>
#include <QDir>
#include <QTemporaryFile>
#include <QSaveFile>
#include <cmath>
#include <climits>

namespace {

const char *const kSysLetters = "GSREJCL"; /**< GNSS (QC) model index order = Sys letters; 'L' = XW1 */
const char *const kRinexSysLetters = "GSREJCI"; /**< Rinex convert letters; 'I' = NavIC (converter SysStr2ExSysMask does not accept 'L') */

/**< Freq-option line keys, used by the loaders' dropped-keys detection: the
 *  GNSS parser's per-system lines plus the Rinex parser's NavIC line, which
 *  has no GNSS branch. */
const char *const kFreqKeys[] = {"GPS", "SBAS", "GLONASS", "Galileo", "QZSS", "BDS", "NavIC", "XW1"};
const int kFreqKeyCount = sizeof(kFreqKeys) / sizeof(kFreqKeys[0]);

QString num(double value)
{
    /**< 6 significant digits: the engine stores these fields as float, so a
     *  12-digit echo of a parsed value would show float noise (0.649999976...). */
    return QString::number(value, 'g', 6);
}

QString numXYZ(double value)
{
    /**< TrueXYZ values pass through the engine as doubles; keep full precision. */
    return QString::number(value, 'g', 12);
}

/**< "1"/"0" text for the engine's boolean keys. */
QString boolText(bool value)
{
    return value ? QStringLiteral("1") : QStringLiteral("0");
}

/**< One "Key=Value# comment" line, mirroring the doc samples. Values are
 *  written without padding: the engine memcpys the raw token including
 *  surrounding spaces into fixed-size fields before any trimming, so a
 *  padded value overflows that field whenever the file is read by the CLI.
 *  An empty value keeps one leading space so the engine's strtok(p, "#")
 *  never hands the comment text itself to the branch. */
QString cfgLine(const char *key, const QString &value, const QString &comment)
{
    if (value.isEmpty())
        return QString("%1= # %2").arg(QString(key).leftJustified(16), comment);
    return QString("%1=%2# %3").arg(QString(key).leftJustified(16), value, comment);
}

/**< Commented-out line for unset times: any line containing '#' before '='
 *  is skipped entirely by the engine parsers. */
QString skippedLine(const char *key, const QString &comment)
{
    return QString("#%1= # %2").arg(QString(key).leftJustified(15), comment);
}

/**< UTC wall-time text for time lines (format shared with the doc samples). */
QString timeValue(const QDateTime &dateTime)
{
    return dateTime.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
}

/**< Active line for a valid time, commented-out line otherwise. */
QString timeLine(const char *key, const QDateTime &dateTime, const QString &comment)
{
    return dateTime.isValid() ? cfgLine(key, timeValue(dateTime), comment)
                              : skippedLine(key, comment);
}

/**< Comma-separated Sys letters in model index order. The two parsers use
 *  different letter sets (QC Str2Sys: ...C,L; converter SysStr2ExSysMask:
 *  ...C,I), so the side must be selected explicitly. */
QString sysLetters(const bool *sysEnabled, bool rinex = false)
{
    const char *const letters = rinex ? kRinexSysLetters : kSysLetters;
    QString out;
    for (int i = 0; i < 7; ++i)
    {
        if (sysEnabled[i]) out.append(QLatin1Char(letters[i]));
    }
    return out;
}

/**< C atoi over the value prefix, mirroring MSVC atoi (= strtol, C locale):
 *  only ASCII whitespace/sign/digits participate -- QChar::isDigit would
 *  also accept full-width digits that the engine's strtol stops at -- and
 *  overflow saturates at INT_MIN/INT_MAX like strtol instead of wrapping
 *  around (a wrap could smuggle a huge engine-side count past this gate). */
int atoiPrefix(const QString &value)
{
    int i = 0;
    while (i < value.size())
    {
        const ushort ch = value.at(i).unicode();
        if (ch != ' ' && (ch < '\t' || ch > '\r')) break;
        ++i;
    }
    bool negative = false;
    if (i < value.size() && (value.at(i).unicode() == '+' || value.at(i).unicode() == '-'))
    {
        negative = (value.at(i).unicode() == '-');
        ++i;
    }
    qint64 result = 0;
    bool saturated = false;
    while (i < value.size())
    {
        const ushort ch = value.at(i).unicode();
        if (ch < '0' || ch > '9') break;
        const int digit = int(ch) - '0';
        if (result > (INT_MAX - digit) / 10)
        {
            saturated = true;
            break;
        }
        result = result * 10 + digit;
        ++i;
    }
    if (saturated) return negative ? INT_MIN : INT_MAX;
    return negative ? static_cast<int>(-result) : static_cast<int>(result);
}

/**< atof over one SNRGroup comma segment, mirroring QCStr2Num: 'd'/'D' act
 *  as exponent markers, and unparsable non-empty text counts as non-zero so
 *  the guard never undercounts the engine's writes. */
double snrSegmentValue(const QString &segment)
{
    if (segment.trimmed().isEmpty()) return 0.0;
    QString text = segment;
    text.replace(QLatin1Char('d'), QLatin1Char('e'));
    text.replace(QLatin1Char('D'), QLatin1Char('E'));
    bool ok = false;
    const double value = text.trimmed().toDouble(&ok);
    return ok ? value : 1.0;
}

/**< Obs2FreId token split set (QCCommon.c): ',', ' ', '\r', '\0', '\n'. */
QStringList freqTokens(const QString &value)
{
    QStringList tokens;
    QString current;
    for (const QChar ch : value)
    {
        if (ch == QLatin1Char(',') || ch == QLatin1Char(' ') || ch == QLatin1Char('\r') || ch == QLatin1Char('\n'))
        {
            if (!current.isEmpty()) tokens << current;
            current.clear();
        }
        else
        {
            current.append(ch);
        }
    }
    if (!current.isEmpty()) tokens << current;
    return tokens;
}

/**< Key classification mirroring the engine's ordered strstr chains in
 *  QCCommon.c (ReadGNSSCfg / ReadRinexCfg): the first table entry whose
 *  text occurs in the key segment decides how the engine consumes the
 *  line, so the prescan walks the tables in exactly the engine order. */
struct EngineKey
{
    const char *pattern; /**< strstr substring, mirrors one engine branch */
    bool safeOnNull;     /**< branch tolerates a NULL value token (bounded strtok(",") loop) */
    bool allFreq;        /**< mirrors the engine AllFreq switch branch */
    int limit;           /**< trimmed-value byte budget for fixed buffers; 0 = unchecked */
};

/**< GNSS branches in engine order (QCCommon.c ReadGNSSCfg). "NavIC" has no
 *  GNSS branch and is therefore absent: such lines are ignored by the
 *  engine and left unchecked here. */
const EngineKey kGnssKeys[] = {
    {"EphMode", false, false, 0},
    {"RinexEphFile", false, false, 0},
    {"Mode", false, false, 0},
    {"Sys", false, false, MAX_SYS_STR_LENGTH - 1},          /**< memcpy into char[MAX_SYS_STR_LENGTH] */
    {"BanPRN", false, false, 0},        /**< Sat2BanSat, bounded writes */
    {"EleDeg", false, false, 0},
    {"Interval", false, false, 0},
    {"SNRGroup", false, false, 0},      /**< Str2SnrGroup writes float[9] unbounded */
    {"IsThresholdSNR", false, false, 0},
    {"IsThresholdMP", false, false, 0},
    {"HeightToReflector", false, false, 0},
    {"RTCMTime", false, false, 0},
    {"StartTime", false, false, 0},
    {"EndTime", false, false, 0},
    {"BaseTrueXYZ", true, false, 0},    /**< strtok(",") loop, bounded */
    {"RoverTrueXYZ", true, false, 0},
    {"IsInputTrueXYZ", false, false, 0},
    {"GPSMpMAX", true, false, 0},
    {"GLOMpMAX", true, false, 0},
    {"GALMpMAX", true, false, 0},
    {"BDSMpMAX", true, false, 0},
    {"QZSMpMAX", true, false, 0},
    {"SBAMpMAX", true, false, 0},
    {"NICMpMAX", true, false, 0},
    {"XW1MpMAX", true, false, 0},
    {"NumGPSSnrMIN", true, false, 0},
    {"NumGLOSnrMIN", true, false, 0},
    {"NumGALSnrMIN", true, false, 0},
    {"NumBDSSnrMIN", true, false, 0},
    {"NumQZSSnrMIN", true, false, 0},
    {"NumSBASnrMIN", true, false, 0},
    {"NumNICSnrMIN", true, false, 0},
    {"NumXW1SnrMIN", true, false, 0},
    {"GPSSnrMIN", true, false, 0},
    {"GLOSnrMIN", true, false, 0},
    {"GALSnrMIN", true, false, 0},
    {"BDSSnrMIN", true, false, 0},
    {"QZSSnrMIN", true, false, 0},
    {"SBASnrMIN", true, false, 0},
    {"NICSnrMIN", true, false, 0},
    {"XW1SnrMIN", true, false, 0},
    {"MpCarrier", false, false, 0},
    {"UseRate", false, false, 0},
    {"CycleJumpRatio", false, false, 0},
    {"MPMethod", false, false, 0},
    {"OutDir", false, false, 0},
    {"RoverFile", false, false, 0},
    {"BaseFile", false, false, 0},
    {"AllFreq", false, true, 0},
    {"GPS", false, false, MAX_FRE_STR_LENGTH - 1},          /**< memcpy into char[MAX_FRE_STR_LENGTH] */
    {"SBAS", false, false, MAX_FRE_STR_LENGTH - 1},
    {"GLONASS", false, false, MAX_FRE_STR_LENGTH - 1},
    {"Galileo", false, false, MAX_FRE_STR_LENGTH - 1},
    {"QZSS", false, false, MAX_FRE_STR_LENGTH - 1},
    {"BDS", false, false, MAX_FRE_STR_LENGTH - 1},
    {"XW1", false, false, MAX_FRE_STR_LENGTH - 1},
    {"IsDoubleDiffCheckLLI", false, false, 0},
    {"IsOutputDataList", false, false, 0},
    {"IsOutputQCLog", false, false, 0},
    {"IsOutputQCResBinaryList", false, false, 0},
    {"RawFileDataType", false, false, 0},
};
const int kGnssKeyCount = sizeof(kGnssKeys) / sizeof(kGnssKeys[0]);

/**< RINEX branches in engine order (QCCommon.c ReadRinexCfg); every branch
 *  dereferences the value token. */
const EngineKey kRinexKeys[] = {
    {"DataType", false, false, 0},
    {"OutputType", false, false, 0},
    {"RinexVer", false, false, 0},
    {"Interval", false, false, 0},
    {"Sys", false, false, MAX_SYS_STR_LENGTH - 1},          /**< char Sys[MAX_SYS_STR_LENGTH] */
    {"SplitTint", false, false, 0},
    {"SplitStart", false, false, 0},
    {"SplitEnd", false, false, 0},
    {"Split", false, false, 0},
    {"RTCMTime", false, false, 0},
    {"CheakSeq", false, false, 0},
    {"LeapSecond", false, false, 0},
    {"AllFreq", false, true, 0},
    {"ObsType", false, false, MAX_OBS_STR_LENGTH - 1},       /**< char ObsType[MAX_OBS_STR_LENGTH] */
    {"GPS", false, false, MAX_FRE_STR_LENGTH - 1},
    {"SBAS", false, false, MAX_FRE_STR_LENGTH - 1},
    {"GLONASS", false, false, MAX_FRE_STR_LENGTH - 1},
    {"Galileo", false, false, MAX_FRE_STR_LENGTH - 1},
    {"QZSS", false, false, MAX_FRE_STR_LENGTH - 1},
    {"BDS", false, false, MAX_FRE_STR_LENGTH - 1},
    {"NavIC", false, false, MAX_FRE_STR_LENGTH - 1},
    {"Pause", false, false, 0},
    {"RawFile", false, false, 0},
    {"OutDir", false, false, 0},
};
const int kRinexKeyCount = sizeof(kRinexKeys) / sizeof(kRinexKeys[0]);

/**< First engine branch whose key pattern occurs in the key segment — the
 *  byte-level mirror of the engine's strstr chain (QCCommon.c
 *  ReadGNSSCfg/ReadRinexCfg). Shared by the prescan classifier and the raw
 *  value extractor so both act on exactly the keys the engine sees. */
const EngineKey *classifyKeySegment(const QByteArray &keySegment, const EngineKey *keys, int keyCount)
{
    for (int i = 0; i < keyCount; ++i)
    {
        if (keySegment.contains(keys[i].pattern)) return &keys[i];
    }
    return nullptr;
}

/**< Leading excerpt of a raw line for error messages: printable ASCII kept
 *  as-is, anything else becomes '?', hard-capped so a hostile line cannot
 *  flood the message box. */
QString printableHead(const QByteArray &line, int maxChars = 40)
{
    const QString text = QString::fromLocal8Bit(line);
    QString head;
    for (int i = 0; i < text.size() && head.size() < maxChars; ++i)
    {
        const ushort ch = text.at(i).unicode();
        head.append((ch >= 0x20 && ch < 0x7F) ? QChar(ch) : QLatin1Char('?'));
    }
    if (text.size() > maxChars) head.append(QLatin1String("..."));
    return head;
}

/**< Byte-level gate in front of the engine parsers: the engine reads
 *  fgets(line[255]) chunks, splits at the first '=' and memcpys raw value
 *  tokens into fixed buffers without clamping, so crafted or damaged files
 *  can overflow it. Every rule mirrors one verified engine behaviour
 *  (QCCommon.c ReadGNSSCfg / ReadRinexCfg); lines are evaluated in engine
 *  order, and both loaders and the save-side self-check run through here. */
bool prescanConfigText(const QByteArray &bytes, bool rinex, QString *errMsg)
{
    const EngineKey *const keys = rinex ? kRinexKeys : kGnssKeys;
    const int keyCount = rinex ? kRinexKeyCount : kGnssKeyCount;
    const QByteArray endMarker = rinex ? QByteArrayLiteral("> END_CHCRINEXCFG")
                                       : QByteArrayLiteral("> END_CHCGNSSCFG");
    int allFreqState = 1;  /**< InitGNSSCfg seeds AllFreq with 1 */

    /**< The engine is pure C-string code (strtok/strstr stop at an embedded
     *  NUL), so a 0x00 byte would make this scan and the engine disagree
     *  about where a line ends; such a file is damaged -- reject it whole. */
    if (bytes.contains('\0'))
    {
        *errMsg = QObject::tr("File contains a NUL byte; the engine would stop reading lines at it.");
        return false;
    }

    int start = 0;
    int lineNo = 0;
    while (start < bytes.size())
    {
        int end = bytes.indexOf('\n', start);
        const bool hasNewline = end >= 0;
        if (!hasNewline) end = bytes.size();
        const QByteArray line = bytes.mid(start, end - start);
        ++lineNo;
        start = hasNewline ? end + 1 : bytes.size();

        /**< fgets reads at most 254 bytes (the 255-byte buffer minus its
         *  terminator) and splits over-long logical lines into independent
         *  chunks: a '#' guard in the first chunk does not protect the rest,
         *  so the chunk cap is evaluated before any other rule. */
        if (line.size() > 254)
        {
            *errMsg = QObject::tr("Line %1 exceeds 254 bytes; the engine reads config lines with a 255-byte buffer: %2")
                          .arg(lineNo).arg(printableHead(line));
            return false;
        }

        /**< strtok(line, "=") key segment: leading '=' skipped, ends at the
         *  next '=' (the '\0' the engine inserts limits every strstr). */
        int keyStart = 0;
        while (keyStart < line.size() && line.at(keyStart) == '=') ++keyStart;
        int keyEnd = keyStart;
        while (keyEnd < line.size() && line.at(keyEnd) != '=') ++keyEnd;
        const QByteArray keySegment = line.mid(keyStart, keyEnd - keyStart);

        if (keySegment.indexOf('#') >= 0) continue;  /**< engine: '#' in the key segment skips the line */
        if (keySegment.indexOf(endMarker) >= 0) break;  /**< engine: section end stops the parser */

        /**< classify: first engine branch whose key occurs in the key segment */
        const EngineKey *match = classifyKeySegment(keySegment, keys, keyCount);
        if (!match) continue;  /**< no engine branch: the line is ignored, harmless */

        /**< strtok(NULL, "=") returns NULL when nothing but '=' follows the
         *  key segment and the engine's fgets chunk for this line carries no
         *  '\n' (a '\n' anywhere in the chunk is itself a non-delimiter, and
         *  a 254-byte line leaves its '\n' for the next chunk); every branch
         *  except the bounded comma loops then dereferences it. The later
         *  strtok(p, "#") cannot crash: it only truncates p in place and its
         *  result is unused by every branch. */
        const bool hasEq = keyEnd < line.size();
        const bool chunkHasNewline = hasNewline && line.size() < 254;
        bool nullToken = !hasEq;
        if (!nullToken && !chunkHasNewline)
        {
            nullToken = true;  /**< nothing but '=' so far after the key */
            for (int i = keyEnd + 1; i < line.size(); ++i)
            {
                if (line.at(i) != '=') { nullToken = false; break; }
            }
        }
        if (nullToken && !match->safeOnNull)
        {
            *errMsg = QObject::tr("Line %1: key \"%2\" carries no value; the engine would read past the line end.")
                          .arg(lineNo).arg(QString::fromLatin1(match->pattern));
            return false;
        }

        /**< value region: after the skipped '='s, cut at the next '=' or '#'. */
        QByteArray value;
        if (hasEq)
        {
            int valueStart = keyEnd + 1;
            while (valueStart < line.size() && line.at(valueStart) == '=') ++valueStart;
            int valueEnd = valueStart;
            while (valueEnd < line.size() && line.at(valueEnd) != '=') ++valueEnd;
            value = line.mid(valueStart, valueEnd - valueStart);
            const int hash = value.indexOf('#');
            if (hash >= 0) value = value.left(hash);
        }

        if (match->limit > 0)
        {
            /**< sanitizeValuesForEngine trims before the engine sees the
             *  line, so the trimmed length is what the buffers receive. */
            const QByteArray trimmed = value.trimmed();
            if (trimmed.size() > match->limit)
            {
                *errMsg = QObject::tr("Line %1: value of %2 exceeds %3 bytes.")
                              .arg(lineNo).arg(QString::fromLatin1(match->pattern)).arg(match->limit);
                return false;
            }
        }

        if (qstrcmp(match->pattern, "SNRGroup") == 0)
        {
            int nonZero = 0;
            const QStringList items = QString::fromLocal8Bit(value).trimmed().split(QLatin1Char(','));
            for (int i = 0; i < items.size(); ++i)
            {
                QString item = items.at(i);
                if (i == items.size() - 1 && !item.isEmpty()) item.chop(1);  /**< engine drops the final character */
                if (snrSegmentValue(item) != 0.0) ++nonZero;
            }
            if (nonZero > QC_MAXELEVINTERV)
            {
                *errMsg = QObject::tr("Line %1: SNRGroup carries more than %2 non-zero values.")
                              .arg(lineNo).arg((int)QC_MAXELEVINTERV);
                return false;
            }
        }

        if (!rinex && match->allFreq)
        {
            allFreqState = atoiPrefix(QString::fromLocal8Bit(value));
        }
        else if (!rinex && match->limit == MAX_FRE_STR_LENGTH - 1 && allFreqState != 1)
        {
            /**< AllFreq==2 routes freq options through Obs2FreId, which writes
             *  char[8] tokens; AllFreq==0 routes them through DecodeFreqList,
             *  which writes SysSigType[6][32] num times without bound. Other
             *  AllFreq values disable both decoders engine-side, so they need
             *  no check -- gating them here too is deliberate strictness. */
            const QString text = QString::fromLocal8Bit(value);
            const QStringList tokens = freqTokens(text);
            for (const QString &token : tokens)
            {
                if (QString(token).toLocal8Bit().size() > MAX_OBS_STR_LENGTH - 1)
                {
                    *errMsg = QObject::tr("Line %1: %2 signal token \"%3\" exceeds 7 bytes when AllFreq is %4.")
                                  .arg(lineNo).arg(QString::fromLatin1(match->pattern), token).arg(allFreqState);
                    return false;
                }
            }
            if (allFreqState == 0)
            {
                const int num = atoiPrefix(text);
                const int commas = text.count(QLatin1Char(','));
                if (num > QC_MAXSIGTYPE || (num > 0 && num > commas))
                {
                    *errMsg = QObject::tr("Line %1: %2 signal count %3 exceeds the %4 decoded frequencies.")
                                  .arg(lineNo).arg(QString::fromLatin1(match->pattern)).arg(num).arg(commas);
                    return false;
                }
            }
        }
    }
    return true;
}

/**< Raw value extraction from the raw file bytes, mirroring the engine's
 *  strtok chain in the byte domain the engine reads: strtok(line, "=")
 *  skips leading '='s before the key, the value token ends at the next '='
 *  (strtok(NULL, "=")) and is then truncated at '#' (strtok(p, "#")).  The
 *  key segment is classified with classifyKeySegment (the engine's strstr
 *  chain), so a line whose key segment contains a branch pattern lands in
 *  that branch's bucket exactly as the engine would consume it; unmatched
 *  keys (e.g. "NavIC", which has no GNSS branch) keep their trimmed text.
 *  Scanning stops at the section end marker, mirroring the engine
 *  (QCCommon.c ReadGNSSCfg/ReadRinexCfg) and prescanConfigText, so lines
 *  after the marker are neither classified nor bucketed.  The input has
 *  already passed prescanConfigText, so the NUL-byte and 254-byte line
 *  checks are not repeated here.  Used for the string keys whose raw text
 *  the engine structs cannot give back (BanPRN, freq options); rows the
 *  engine skips ('#' in the key segment) are skipped here too. */
void extractRawValues(const QByteArray &bytes, bool rinex, QHash<QString, QString> *out)
{
    const EngineKey *const keys = rinex ? kRinexKeys : kGnssKeys;
    const int keyCount = rinex ? kRinexKeyCount : kGnssKeyCount;
    const QByteArray endMarker = rinex ? QByteArrayLiteral("> END_CHCRINEXCFG")
                                       : QByteArrayLiteral("> END_CHCGNSSCFG");
    int start = 0;
    while (start < bytes.size())
    {
        const int newline = bytes.indexOf('\n', start);
        const int end = (newline < 0) ? bytes.size() : newline;
        const QByteArray line = bytes.mid(start, end - start);
        start = (newline < 0) ? bytes.size() : newline + 1;

        int keyStart = 0;
        while (keyStart < line.size() && line.at(keyStart) == '=') ++keyStart;
        int keyEnd = keyStart;
        while (keyEnd < line.size() && line.at(keyEnd) != '=') ++keyEnd;
        const QByteArray keySegment = line.mid(keyStart, keyEnd - keyStart);

        if (keySegment.indexOf('#') >= 0) continue;  /**< engine: '#' in the key segment skips the line */
        if (keySegment.indexOf(endMarker) >= 0) break;  /**< engine: section end stops the parser */
        if (keyEnd == line.size()) continue;  /**< no '=': no value token to extract */

        const int eq = keyEnd;
        int valueStart = eq + 1;
        while (valueStart < line.size() && line.at(valueStart) == '=') ++valueStart;
        int valueEnd = valueStart;
        while (valueEnd < line.size() && line.at(valueEnd) != '=') ++valueEnd;
        QByteArray value = line.mid(valueStart, valueEnd - valueStart);
        const int valueHash = value.indexOf('#');
        if (valueHash >= 0) value.truncate(valueHash);

        const EngineKey *match = classifyKeySegment(keySegment, keys, keyCount);
        const QString key = match ? QString::fromLatin1(match->pattern)
                                  : QString::fromLocal8Bit(keySegment).trimmed();
        out->insert(key, QString::fromLocal8Bit(value).trimmed());
    }
}

QDateTime qctimeToDateTime(const QC_TIME_T &t)
{
    if (t.Time == 0) return QDateTime();
    /**< Epoch2QCTime treats the fields as a plain 1970-based calendar, i.e.
     *  UTC wall time. Using UTC here keeps file text <-> form values stable
     *  (LocalTime would shift the value by the timezone offset every load). */
    return QDateTime::fromSecsSinceEpoch((qint64)t.Time, Qt::UTC);
}

/**< Trim every value (text between '=' and '#') so the engine's unclamped
 *  memcpy cannot overflow on padded values: the shipped doc sample pads its
 *  ObsType line to 9 bytes in an 8-byte field. Trimmed values are what the
 *  engine semantically consumes anyway (AdjustStr trims most fields).
 *  Leading '='s are stripped first because strtok(line, "=") skips them
 *  before the key, so a padded value behind such a prefix would otherwise
 *  reach the engine untouched. */
QString sanitizeValuesForEngine(const QString &text)
{
    QStringList lines;
    for (const QString &line : text.split(QLatin1Char('\n')))
    {
        int keyStart = 0;
        while (keyStart < line.size() && line.at(keyStart) == QLatin1Char('=')) ++keyStart;
        const QString stripped = line.mid(keyStart);
        const int eq = stripped.indexOf(QLatin1Char('='));
        const int hash = stripped.indexOf(QLatin1Char('#'));
        if (eq <= 0 || (hash >= 0 && hash < eq))
        {
            lines << stripped;
            continue;
        }
        const QString value = stripped.mid(eq + 1, hash >= 0 ? hash - eq - 1 : -1);
        /**< Non-empty values are written without a leading space: a padded 8-byte
         *  value would fill the ObsType char[8] completely and lose its NUL
         *  terminator. An empty value keeps one space so the engine's
         *  strtok(p, "#") never returns the comment text itself. */
        const QString trimmed = value.trimmed();
        QString rebuilt;
        if (trimmed.isEmpty())
        {
            rebuilt = stripped.left(eq + 1) + QLatin1Char(' ');
        }
        else
        {
            rebuilt = stripped.left(eq + 1) + trimmed;
        }
        if (hash >= 0) rebuilt += stripped.mid(hash);
        lines << rebuilt;
    }
    return lines.join(QLatin1Char('\n'));
}

bool parseTextWithEngine(const QString &text, bool rinex, CHCQC_GNSSCFG_T *gnssCfg,
                         CHCQC_RINEXCFG_T *rinexCfg, QString *errMsg)
{
    /**< The engine API reads from a FILE*, so hand it a trimmed temporary copy. */
    QTemporaryFile temp;
    if (!temp.open())
    {
        *errMsg = QObject::tr("Cannot create temporary file: %1").arg(temp.errorString());
        return false;
    }
    temp.write(sanitizeValuesForEngine(text).toLocal8Bit());
    const QString tempPath = temp.fileName();
    temp.close();
    if (temp.error() != QFile::NoError)
    {
        *errMsg = QObject::tr("Cannot create temporary file: %1").arg(temp.errorString());
        return false;
    }

    FILE *fp = fopen(PathUtils::toLocalFilePathBytes(tempPath).constData(), "rb");
    if (!fp)
    {
        *errMsg = QObject::tr("Cannot open file: %1").arg(tempPath);
        return false;
    }
    if (rinex)
    {
        InitRinexCfg(rinexCfg);
        ReadRinexCfg(fp, rinexCfg);
    }
    else
    {
        InitGNSSCfg(gnssCfg);
        ReadGNSSCfg(fp, gnssCfg);
    }
    fclose(fp);
    return true;
}

/**< Local-8-bit view of the engine's char[] string fields. */
QString fromCfgText(const char *text)
{
    return QString::fromLocal8Bit(text);
}

/**< Reads the whole file; missing/unreadable files surface through @p errMsg,
 *  and so does a mid-read failure (e.g. a flaky network share), which would
 *  otherwise pass a silently truncated file on as a valid-looking config. */
bool readConfigBytes(const QString &path, QByteArray *bytes, QString *errMsg)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        *errMsg = QObject::tr("Cannot open file: %1").arg(path);
        return false;
    }
    *bytes = file.readAll();
    if (file.error() != QFile::NoError)
    {
        *errMsg = QObject::tr("Cannot read file: %1 (%2)").arg(path, file.errorString());
        return false;
    }
    return true;
}

/**< Writes the prescan-approved bytes through a QSaveFile (write to a
 *  temporary sibling, then atomically rename over the target) so a failure
 *  at any stage — full disk, short write, failed commit — can never
 *  truncate or destroy the original file, which may be the user's only
 *  copy of the config. */
bool writeConfigText(const QString &path, const QByteArray &bytes, QString *errMsg)
{
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
    {
        *errMsg = QObject::tr("Cannot write file: %1 (%2)").arg(path, file.errorString());
        return false;
    }
    if (file.write(bytes) != bytes.size())
    {
        *errMsg = QObject::tr("Cannot write file: %1 (%2)").arg(path, file.errorString());
        return false;
    }
    if (!file.commit())
    {
        *errMsg = QObject::tr("Cannot write file: %1 (%2)").arg(path, file.errorString());
        return false;
    }
    return true;
}

/**< Budget guard for path-like values. The budget is the per-key remainder
 *  of the written line: cfgLine produces key(16) + '=' + value + "# " +
 *  comment (+ '\n') and fgets(255) must read that in one chunk, so
 *  value <= 234 - comment bytes (budgets see the comment literals in the
 *  save functions). The '='/'#' ban keeps the value from splitting the
 *  line the engine reads. */
bool checkPathValue(const QString &value, const char *key, int budget, QString *errMsg)
{
    if (value.contains(QLatin1Char('=')) || value.contains(QLatin1Char('#')))
    {
        *errMsg = QObject::tr("%1 must not contain '=' or '#'.").arg(key);
        return false;
    }
    if (QString(value).toLocal8Bit().size() > budget)
    {
        *errMsg = QObject::tr("%1 is too long (max %2 bytes).").arg(key).arg(budget);
        return false;
    }
    return true;
}

/**< True when at least one entry of @p sysEnabled is set. */
bool anySysEnabled(const bool *sysEnabled)
{
    for (int i = 0; i < 7; ++i)
    {
        if (sysEnabled[i]) return true;
    }
    return false;
}

/**< Appends @p message to @p warnMsg with '\n' separators. */
void appendWarn(QString *warnMsg, const QString &message)
{
    if (!warnMsg) return;
    if (!warnMsg->isEmpty()) warnMsg->append(QLatin1Char('\n'));
    *warnMsg += message;
}

/**< Appends the per-system freq-option keys whose raw bucket is non-empty:
 *  the freq branches leave no trace in the engine structs, and empty values
 *  are not "specified frequencies". */
void appendDroppedFreqKeys(const QHash<QString, QString> &raw, QStringList *dropped)
{
    for (int i = 0; i < kFreqKeyCount; ++i)
    {
        if (!raw.value(QLatin1String(kFreqKeys[i])).isEmpty())
            *dropped << QString::fromLatin1(kFreqKeys[i]);
    }
}

/**< Message for keys the editors no longer write (frequency options, file
 *  paths, ObsType): saving drops the lines and AllFreq is always written as
 *  1, so the loaders surface exactly what a save would remove. */
QString droppedKeysNoteText(const QStringList &keys)
{
    return QObject::tr("The file contains %1 settings the editor no longer writes; "
                       "saving drops these lines (frequencies are always written as all frequencies).")
        .arg(keys.join(QStringLiteral(", ")));
}

} // namespace

namespace QCConfigFileModel {

GnssCfgFileData defaultGnssCfgFileData()
{
    GnssCfgFileData d;
    for (int i = 0; i < 7; ++i) d.sysEnabled[i] = true;

    /**< Threshold defaults mirror the main QC settings dialog (s_defaultCheckConfigData):
     *  MaxMP 0.5 [m] on every frequency and MinSNR 38 [dBHz] on the first
     *  frequency / 35 on the rest, expanded per system exactly the way
     *  generateAPIConfig broadcasts the dialog values to the engine.
     *  XW1 (row 6) keeps the engine InitGNSSCfg defaults (45 dBHz) instead —
     *  product decision, 2026-09-17. NavIC (row 7) has no Sys letter; its
     *  NIC* keys are still written for round-trip fidelity although the
     *  engine no longer consumes them. */
    static const double kMpMax[8][6] = {
        {0.5, 0.5, 0.5, 0.5, 0.5},            // GPS     (5)
        {0.5, 0.5},                           // SBAS    (2)
        {0.5, 0.5},                           // GLONASS (2)
        {0.5, 0.5, 0.5, 0.5, 0.5},            // Galileo (5)
        {0.5, 0.5, 0.5, 0.5, 0.5, 0.5},       // QZSS    (6)
        {0.5, 0.5, 0.5, 0.5, 0.5, 0.5},       // BDS     (6)
        {0.5, 0.5, 0.5, 0.5},                 // XW1     (4)
        {0.5, 0.5},                           // NavIC   (2)
    };
    static const int kSnrMin[8][6] = {
        {38, 35, 35, 35, 35},                 // GPS
        {38, 35},                             // SBAS
        {38, 35},                             // GLONASS
        {38, 35, 35, 35, 35},                 // Galileo
        {38, 35, 35, 35, 35, 35},             // QZSS
        {38, 35, 35, 35, 35, 35},             // BDS
        {45, 45, 45, 45},                     // XW1 (engine default)
        {38, 35},                             // NavIC
    };
    for (int i = 0; i < 8; ++i)
    {
        const int n = systemFreqCount(i);
        d.mpMax[i].clear();
        d.snrMin[i].clear();
        d.numSnrMin[i].clear();
        for (int k = 0; k < n; ++k)
        {
            d.mpMax[i] << kMpMax[i][k];
            d.snrMin[i] << kSnrMin[i][k];
            d.numSnrMin[i] << 2;
        }
    }
    d.snrGroup = {30.0, 90.0};
    d.baseTrueXYZ = {0.0, 0.0, 0.0};
    d.roverTrueXYZ = {0.0, 0.0, 0.0};
    /**< InitGNSSCfg leaves Mode/Interval/UseRate/CJR/MpCarrier/IsThesholdSNR/
     *  IsThesholdMP/HtoReflector/RTCMTime/SNRGroupSize uninitialised, so the
     *  "new file" defaults are curated here instead (doc sample values). */
    d.mode = -1;
    d.interval = 1.0;
    d.isThresholdSNR = true;
    d.isThresholdMP = true;
    d.useRate = 95.0;
    d.cycleJumpRatio = 400;
    d.mpCarrier = 0.005;
    return d;
}

RinexCfgFileData defaultRinexCfgFileData()
{
    RinexCfgFileData d;
    for (int i = 0; i < 7; ++i) d.sysEnabled[i] = true;
    d.dataType = 11;   /**< CLI falls back to RINEX when no format is given */
    d.rinexVersion = 304;
    return d;
}

bool loadGnssCfgFile(const QString &path, GnssCfgFileData *out, QString *errMsg, QString *droppedKeysNote, QString *sysNote)
{
    QByteArray bytes;
    if (!readConfigBytes(path, &bytes, errMsg)) return false;
    if (!prescanConfigText(bytes, false, errMsg)) return false;
    const QString text = QString::fromLocal8Bit(bytes);
    if (!text.contains(QStringLiteral("> START_CHCGNSSCFG")))
    {
        *errMsg = QObject::tr("Not a GNSS QC config file (section marker \"> START_CHCGNSSCFG\" missing): %1").arg(path);
        return false;
    }
    QHash<QString, QString> raw;
    extractRawValues(bytes, false, &raw);

    CHCQC_GNSSCFG_T cfg = {}; /**< zero-init mirrors the CLI loader, then engine defaults */
    if (!parseTextWithEngine(text, false, &cfg, nullptr, errMsg)) return false;

    out->mode = cfg.Mode;
    out->rawFileDataType = cfg.RawFileDataType;
    static const unsigned int kSysBits[7] = {
        QC_SYS_GPS, QC_SYS_SBS, QC_SYS_GLO, QC_SYS_GAL, QC_SYS_QZS, QC_SYS_CMP, QC_SYS_XW1,
    };
    for (int i = 0; i < 7; ++i) out->sysEnabled[i] = (cfg.Sys & kSysBits[i]) != 0;
    /**< Legacy 'I' migration: the QC parser's Str2Sys no longer accepts 'I'
     *  (it used to set the 0x80 bit). "Sys=G,C,I" reads back without the
     *  bit, so the migrated state is restored explicitly; "Sys=I" alone
     *  made Str2Sys return 255 (all systems), which already checked every
     *  box — the explicit set is a no-op there, only the note differs. */
    const QString rawSys = raw.value(QStringLiteral("Sys"));
    if (rawSys.contains(QLatin1Char('I'), Qt::CaseInsensitive))
    {
        out->sysEnabled[6] = true;
        if (sysNote)
        {
            *sysNote = QObject::tr("The Sys line uses the letter I, which the QC engine no longer accepts; "
                                   "saving writes it as L (XW1). A Sys line containing only I was already treated by the engine as all systems.");
        }
    }
    out->banPrn = raw.value(QStringLiteral("BanPRN"));
    out->eleDeg = cfg.EleDeg;
    out->interval = cfg.Interval;
    out->snrGroup.clear();
    const int snrGroupSize = qBound(0, (int)cfg.SNRGroupSize, (int)QC_MAXELEVINTERV);
    for (int i = 0; i < snrGroupSize; ++i) out->snrGroup << cfg.SNRGroup[i];
    out->isThresholdSNR = cfg.IsThesholdSNR != 0;
    out->isThresholdMP = cfg.IsThesholdMP != 0;
    out->heightToReflector = cfg.HtoReflector;
    out->rtcmTime = qctimeToDateTime(cfg.RTCMTime);
    out->startTime = qctimeToDateTime(cfg.StartTime);
    out->endTime = qctimeToDateTime(cfg.EndTime);
    out->baseTrueXYZ.clear();
    out->roverTrueXYZ.clear();
    for (int i = 0; i < 3; ++i)
    {
        out->baseTrueXYZ << cfg.BaseTrueXYZ[i];
        out->roverTrueXYZ << cfg.RoverTrueXYZ[i];
    }
    out->isInputTrueXYZ = cfg.IsInputTrueXYZ != 0;
    out->isOutputDataList = cfg.bIsOutputDataList != 0;
    out->isOutputQCLog = cfg.bIsOutputQCLog != 0;
    out->isOutputQCResBinaryList = cfg.bIsOutputQCResBinaryList != 0;
    out->ephMode = cfg.EphMode;
    out->rinexEphFile = fromCfgText(cfg.RinexEphFile);
    out->mpMethod = cfg.MPMethod;

    /**< Rows 0..5 pair with the engine arrays G,S,R,E,J,C; row 6 is XW1
     *  (Sys letter 'L', engine LMpMAX family) and row 7 is NavIC (no Sys
     *  letter, NIC* keys parsed into IMpMAX but no longer consumed). */
    const float *mpArrays[8] = {cfg.GMpMAX, cfg.SMpMAX, cfg.RMpMAX, cfg.EMpMAX, cfg.JMpMAX, cfg.CMpMAX, cfg.LMpMAX, cfg.IMpMAX};
    const unsigned int *snrArrays[8] = {cfg.GSnrMIN, cfg.SSnrMIN, cfg.RSnrMIN, cfg.ESnrMIN, cfg.JSnrMIN, cfg.CSnrMIN, cfg.LSnrMIN, cfg.ISnrMIN};
    const unsigned int *numSnrArrays[8] = {cfg.NumGSnrMIN, cfg.NumSSnrMIN, cfg.NumRSnrMIN, cfg.NumESnrMIN, cfg.NumJSnrMIN, cfg.NumCSnrMIN, cfg.NumLSnrMIN, cfg.NumISnrMIN};
    for (int i = 0; i < 8; ++i)
    {
        const int n = systemFreqCount(i);
        out->mpMax[i].clear();
        out->snrMin[i].clear();
        out->numSnrMin[i].clear();
        for (int k = 0; k < n; ++k)
        {
            out->mpMax[i] << mpArrays[i][k];
            out->snrMin[i] << (int)snrArrays[i][k];
            out->numSnrMin[i] << (int)numSnrArrays[i][k];
        }
    }
    out->mpCarrier = cfg.MPCarrier;
    out->useRate = cfg.UseRate;
    out->cycleJumpRatio = cfg.CJR;
    out->isDoubleDiffCheckLLI = cfg.bIsDoubleDiffCheckLLI != 0;

    /**< Dropped-keys note: the engine only fills its path fields inside their
     *  own branches, so non-empty means the line exists; the GNSS freq
     *  branches decode into engine-side tables and leave no struct trace, so
     *  the raw buckets are the only witness. Empty freq values are not
     *  "specified frequencies" and stay silent. */
    if (droppedKeysNote)
    {
        QStringList dropped;
        if (cfg.AllFreq != 1) dropped << QStringLiteral("AllFreq");
        appendDroppedFreqKeys(raw, &dropped);
        if (cfg.OutDir[0] != '\0') dropped << QStringLiteral("OutDir");
        if (cfg.RoverFile[0] != '\0') dropped << QStringLiteral("RoverFile");
        if (cfg.BaseFile[0] != '\0') dropped << QStringLiteral("BaseFile");
        if (!dropped.isEmpty()) *droppedKeysNote = droppedKeysNoteText(dropped);
    }
    return true;
}

bool loadRinexCfgFile(const QString &path, RinexCfgFileData *out, QString *errMsg, QString *droppedKeysNote)
{
    QByteArray bytes;
    if (!readConfigBytes(path, &bytes, errMsg)) return false;
    if (!prescanConfigText(bytes, true, errMsg)) return false;
    const QString text = QString::fromLocal8Bit(bytes);
    if (!text.contains(QStringLiteral("> START_CHCRINEXCFG")))
    {
        *errMsg = QObject::tr("Not a Rinex convert config file (section marker \"> START_CHCRINEXCFG\" missing): %1").arg(path);
        return false;
    }
    QHash<QString, QString> raw;
    extractRawValues(bytes, true, &raw);

    CHCQC_RINEXCFG_T cfg = {};
    if (!parseTextWithEngine(text, true, nullptr, &cfg, errMsg)) return false;

    out->dataType = (int)cfg.DataType;
    out->outputType = (int)cfg.OutputType;
    out->rinexVersion = (int)cfg.RinexVer;
    out->interval = cfg.Interval;
    const QString sysText = fromCfgText((const char *)cfg.Sys);
    for (int i = 0; i < 7; ++i)
    {
        out->sysEnabled[i] = sysText.contains(QLatin1Char(kRinexSysLetters[i]), Qt::CaseInsensitive);
    }
    /**< Mirror of the GNSS-side 'I' migration, but for the letter the Rinex
     *  converter actually drops: 'L' is not accepted by SysStr2ExSysMask,
     *  so a file carrying it silently loses the entry on save. */
    const QString sysLDroppedNote = sysText.contains(QLatin1Char('L'), Qt::CaseInsensitive)
        ? QObject::tr("The Sys line contains the letter L, which the Rinex converter does not support; saving drops it.")
        : QString();
    out->split = cfg.Split;
    out->splitTimeInt = cfg.SplitTimeInt;
    out->splitStart = qctimeToDateTime(cfg.SplitStart);
    out->splitEnd = qctimeToDateTime(cfg.SplitEnd);
    out->rtcmTime = qctimeToDateTime(cfg.RTCMTime);
    out->cheakSeq = cfg.CheakSeq != 0;
    out->leapSecond = cfg.LeapSecond;
    out->pause = cfg.Pause != 0;

    /**< Dropped-keys note: AllFreq has no InitRinexCfg seed (unlike
     *  InitGNSSCfg's 1), so a missing line reads back as 0 — the raw
     *  bucket is the only way to tell "line absent" from "explicitly
     *  != 1". Empty values count as unspecified everywhere, mirroring
     *  the GNSS side. */
    if (droppedKeysNote)
    {
        QStringList dropped;
        if (!raw.value(QStringLiteral("AllFreq")).isEmpty() && cfg.AllFreq != TRUE) dropped << QStringLiteral("AllFreq");
        appendDroppedFreqKeys(raw, &dropped);
        if (!raw.value(QStringLiteral("ObsType")).isEmpty()) dropped << QStringLiteral("ObsType");
        if (!raw.value(QStringLiteral("RawFile")).isEmpty()) dropped << QStringLiteral("RawFile");
        if (!raw.value(QStringLiteral("OutDir")).isEmpty()) dropped << QStringLiteral("OutDir");
        QStringList notes;
        if (!dropped.isEmpty()) notes << droppedKeysNoteText(dropped);
        if (!sysLDroppedNote.isEmpty()) notes << sysLDroppedNote;
        if (!notes.isEmpty()) *droppedKeysNote = notes.join(QLatin1Char('\n'));
    }
    return true;
}

bool saveGnssCfgFile(const QString &path, const GnssCfgFileData &d, QString *errMsg)
{
    /**< Warnings are the dialog's business (it runs the same validate and asks
     *  before saving); the save path only blocks on errors. */
    if (!validateGnssCfgFileData(d, errMsg, nullptr)) return false;

    QStringList body;
    body << QStringLiteral("> START_CHCGNSSCFG");
    body << cfgLine("Mode", QString::number(d.mode), "GNSS QC mode(-1:NONE 0:zero baseline 1:diff sat and epoch 2:short baseline 3:tripleDiff epoch)");
    body << cfgLine("Sys", sysLetters(d.sysEnabled), "gnss system choose(GPS:G,SBAS:S,GLO:R,GAL:E,QZS:J,BDS:C,XW1:L)");
    body << cfgLine("RawFileDataType", QString::number(d.rawFileDataType), "File Data type, -1-auto, 0-RTCM2, 1-RTCM3, 11-Rinex, 37-HRCX, 38-HRC, 51-RawMsg ASCII, 52-RawMsg BIN");
    body << cfgLine("BanPRN", d.banPrn, "ban satellite group(G23,C01...)");
    body << cfgLine("EleDeg", QString::number(d.eleDeg), "[deg] ele mask");
    body << cfgLine("Interval", num(d.interval), "[s] sample interval");
    /**< The engine parser drops the last character of the final SNRGroup
     *  item ("30" reads back as {3}), so the list is written with a trailing
     *  comma to keep the parsed values stable across load/save cycles. */
    QString snrGroupText = joinDoubles(d.snrGroup);
    if (!snrGroupText.isEmpty()) snrGroupText += QLatin1Char(',');
    body << cfgLine("SNRGroup", snrGroupText, "[deg] grouping SNR with ele(0,10,30,40,90)");
    body << cfgLine("IsThresholdSNR", boolText(d.isThresholdSNR), "whether to verify SNR threshold");
    body << cfgLine("IsThresholdMP", boolText(d.isThresholdMP), "whether to verify MP threshold");
    body << cfgLine("HeightToReflector", num(d.heightToReflector), "[m] height from main reflector");
    body << timeLine("RTCMTime", d.rtcmTime, "when decoding RTCM data, the time of the day");
    body << timeLine("StartTime", d.startTime, "start time to process");
    body << timeLine("EndTime", d.endTime, "end time to process");
    body << cfgLine("BaseTrueXYZ", joinXYZ(d.baseTrueXYZ), "[m] base ecef coordinate");
    body << cfgLine("RoverTrueXYZ", joinXYZ(d.roverTrueXYZ), "[m] rover ecef coordinate");
    body << cfgLine("IsOutputDataList", boolText(d.isOutputDataList), "whether to output DataList file");
    body << cfgLine("IsOutputQCLog", boolText(d.isOutputQCLog), "whether to output .CS/.MP file");
    body << cfgLine("IsOutputQCResBinaryList", boolText(d.isOutputQCResBinaryList), "whether to output _QCResList.txt file");
    body << cfgLine("EphMode", QString::number(d.ephMode), "ephemeris mode: 0-auto find same-name eph, 1-no ephemeris, 2-specified eph file");
    body << cfgLine("RinexEphFile", d.rinexEphFile, "specified RINEX eph/nav file path when EphMode=2");
    body << cfgLine("MPMethod", QString::number(d.mpMethod), "MP process method, 0: fix 50-epoch, 1: filter");
    /**< The product has no frequency selection, so the value is fixed at 1
     *  (all frequencies) and no per-system option lines are written. */
    body << cfgLine("AllFreq", QStringLiteral("1"), "gnss observation all frequency (0:from user 1:all freq 2:simple from user)");
    body << cfgLine("IsInputTrueXYZ", boolText(d.isInputTrueXYZ), "whether to input base and rover XYZ");
    body << cfgLine("IsDoubleDiffCheckLLI", boolText(d.isDoubleDiffCheckLLI), "whether double difference between station and satellite takes into LLI");
    /**< Rows 0..5 = GPS..BDS, 6 = XW1 (XW1* keys), 7 = NavIC (NIC* keys,
     *  engine-parsed but not consumed — kept for round-trip fidelity). */
    static const char *const kMpKeys[8] = {"GPSMpMAX", "SBAMpMAX", "GLOMpMAX", "GALMpMAX", "QZSMpMAX", "BDSMpMAX", "XW1MpMAX", "NICMpMAX"};
    static const char *const kSnrKeys[8] = {"GPSSnrMIN", "SBASnrMIN", "GLOSnrMIN", "GALSnrMIN", "QZSSnrMIN", "BDSSnrMIN", "XW1SnrMIN", "NICSnrMIN"};
    static const char *const kNumSnrKeys[8] = {"NumGPSSnrMIN", "NumSBASnrMIN", "NumGLOSnrMIN", "NumGALSnrMIN", "NumQZSSnrMIN", "NumBDSSnrMIN", "NumXW1SnrMIN", "NumNICSnrMIN"};
    for (int i = 0; i < 8; ++i)
    {
        body << cfgLine(kMpKeys[i], joinDoubles(d.mpMax[i]), QStringLiteral("[m] max MP Rms (%1)").arg(systemFreqNames(i)));
    }
    body << cfgLine("MpCarrier", num(d.mpCarrier), "[m] max MP Rms carrier");
    for (int i = 0; i < 8; ++i)
    {
        body << cfgLine(kSnrKeys[i], joinInts(d.snrMin[i]), QStringLiteral("[dBHz] min SNR (%1)").arg(systemFreqNames(i)));
    }
    body << cfgLine("UseRate", num(d.useRate), "[%] data use rate");
    body << cfgLine("CycleJumpRatio", QString::number(d.cycleJumpRatio), "cycle jump ratio");
    for (int i = 0; i < 8; ++i)
    {
        body << cfgLine(kNumSnrKeys[i], joinInts(d.numSnrMin[i]), QStringLiteral("min SNR sat number (%1)").arg(systemFreqNames(i)));
    }
    body << QStringLiteral("> END_CHCGNSSCFG");
    const QByteArray encoded = (body.join(QLatin1Char('\n')) + QLatin1Char('\n')).toLocal8Bit();
    /**< Same gate as the loader: the written file must survive the prescan so
     *  the editor can reopen everything it produces (and the CLI reads the
     *  same bytes). */
    if (!prescanConfigText(encoded, false, errMsg)) return false;
    return writeConfigText(path, encoded, errMsg);
}

bool saveRinexCfgFile(const QString &path, const RinexCfgFileData &d, QString *errMsg)
{
    if (!validateRinexCfgFileData(d, errMsg, nullptr)) return false;

    QStringList body;
    body << QStringLiteral("> START_CHCRINEXCFG");
    body << cfgLine("DataType", QString::number(d.dataType), "input file type (see GNSS_STRFMT_xx)");
    body << cfgLine("OutputType", QString::number(d.outputType), "output file type, 0-RINEX, 1-RawMsg ASCII, 2-BaseObs ASCII, 3-RawMsg process, 4-RTCM, 5-RTKPLOT, 6-ASCII, 7-BINARY");
    body << cfgLine("RinexVer", rinexVersionText(d.rinexVersion), "rinex file version");
    body << cfgLine("Interval", num(d.interval), "[s] sample interval(0:from raw file, x>0:from user)");
    body << cfgLine("Sys", sysLetters(d.sysEnabled, true), "gnss system choose(GPS:G,SBAS:S,GLO:R,GAL:E,QZS:J,BDS:C,NavIC:I)");
    body << cfgLine("Split", QString::number(d.split), "obsfile split option(0:no,1:station moving,2:time interval,3:time period)");
    body << cfgLine("SplitTint", QString::number(d.splitTimeInt), "[s] time interval to use when splitting files");
    body << timeLine("SplitStart", d.splitStart, "start time when splitting files");
    body << timeLine("SplitEnd", d.splitEnd, "end time when splitting files");
    body << timeLine("RTCMTime", d.rtcmTime, "when decoding RTCM data, the time of the day");
    /**< The product has no frequency or obs-type selection: AllFreq is fixed
     *  at 1 (all frequencies) and no ObsType / per-system option lines are
     *  written (the engine never consumes ObsType anyway). */
    body << cfgLine("AllFreq", QStringLiteral("1"), "gnss observation all frequency (0:from user 1:all freq)");
    body << cfgLine("Pause", boolText(d.pause), "whether to pause at the end(0:no,1:yes)");
    body << QStringLiteral("> END_CHCRINEXCFG");
    const QByteArray encoded = (body.join(QLatin1Char('\n')) + QLatin1Char('\n')).toLocal8Bit();
    /**< Same gate as the loader: the written file must survive the prescan so
     *  the editor can reopen everything it produces. */
    if (!prescanConfigText(encoded, true, errMsg)) return false;
    return writeConfigText(path, encoded, errMsg);
}

bool validateGnssCfgFileData(const GnssCfgFileData &d, QString *errMsg, QString *warnMsg)
{
    if (!anySysEnabled(d.sysEnabled))
    {
        *errMsg = QObject::tr("At least one GNSS system must be selected (Sys).");
        return false;
    }
    if (d.snrGroup.size() > (int)QC_MAXELEVINTERV)
    {
        *errMsg = QObject::tr("SNRGroup allows at most %1 values.").arg((int)QC_MAXELEVINTERV);
        return false;
    }
    for (double v : d.snrGroup)
    {
        if (!std::isfinite(v) || v < 0.0)
        {
            *errMsg = QObject::tr("SNRGroup values must be non-negative numbers.");
            return false;
        }
    }
    for (int i = 0; i < 8; ++i)
    {
        const int n = systemFreqCount(i);
        if (d.mpMax[i].size() != n || d.snrMin[i].size() != n || d.numSnrMin[i].size() != n)
        {
            *errMsg = QObject::tr("%1 thresholds require exactly %2 values.").arg(systemLabel(i)).arg(n);
            return false;
        }
        for (int k = 0; k < n; ++k)
        {
            if (!std::isfinite(d.mpMax[i][k]) || d.mpMax[i][k] < 0.0 || d.snrMin[i][k] < 0 || d.numSnrMin[i][k] < 0)
            {
                *errMsg = QObject::tr("%1 threshold values must be non-negative numbers.").arg(systemLabel(i));
                return false;
            }
        }
    }
    if (d.startTime.isValid() && d.endTime.isValid() && d.startTime > d.endTime)
    {
        *errMsg = QObject::tr("Start time must not be later than end time.");
        return false;
    }
    if (!checkPathValue(d.rinexEphFile, "RinexEphFile", 186, errMsg)) return false;
    if (!checkPathValue(d.banPrn, "BanPRN", 203, errMsg)) return false;
    /**< TrueXYZ values are consumed as raw doubles by the baseline solver,
     *  so non-finite text (e.g. "1e400", which toDouble accepts) must never
     *  reach the file. */
    for (int i = 0; i < 3; ++i)
    {
        if (!std::isfinite(d.baseTrueXYZ[i]) || !std::isfinite(d.roverTrueXYZ[i]))
        {
            *errMsg = QObject::tr("Base/Rover TrueXYZ values must be finite numbers.");
            return false;
        }
    }
    if (d.isInputTrueXYZ)
    {
        bool allZero = true;
        for (int i = 0; i < 3; ++i) allZero = allZero && d.baseTrueXYZ[i] == 0.0 && d.roverTrueXYZ[i] == 0.0;
        if (allZero) appendWarn(warnMsg, QObject::tr("IsInputTrueXYZ is enabled but Base/RoverTrueXYZ are all zero."));
    }
    return true;
}

bool validateRinexCfgFileData(const RinexCfgFileData &d, QString *errMsg, QString *warnMsg)
{
    Q_UNUSED(warnMsg);
    if (!anySysEnabled(d.sysEnabled))
    {
        *errMsg = QObject::tr("At least one system must be selected (Sys); the engine treats an empty Sys as all systems.");
        return false;
    }
    if (d.splitStart.isValid() && d.splitEnd.isValid() && d.splitStart > d.splitEnd)
    {
        *errMsg = QObject::tr("Split start time must not be later than split end time.");
        return false;
    }
    return true;
}

QString rinexVersionText(int rinexVersion)
{
    switch (rinexVersion)
    {
    case 211: return QStringLiteral("2.11");
    case 301: return QStringLiteral("3.01");
    case 302: return QStringLiteral("3.02");
    case 303: return QStringLiteral("3.03");
    case 304: return QStringLiteral("3.04");
    case 400: return QStringLiteral("4.00");
    default:  return QString::number(rinexVersion / 100.0, 'f', 2);
    }
}

int systemFreqCount(int systemIndex)
{
    switch (systemIndex)
    {
    case 0: return QC_GPS_FRE_NUM;      /**< GPS */
    case 1: return QC_SBAS_FRE_NUM;     /**< SBAS */
    case 2: return QC_GLONASS_FRE_NUM;  /**< GLONASS */
    case 3: return QC_GALILEO_FRE_NUM;  /**< Galileo */
    case 4: return QC_QZSS_FRE_NUM;     /**< QZSS */
    case 5: return QC_BDS_FRE_NUM;      /**< BDS */
    case 6: return QC_XW1_FRE_NUM;      /**< XW1 */
    default: return QC_IRNSS_FRE_NUM;   /**< NavIC (no Sys letter) */
    }
}

QString systemFreqNames(int systemIndex)
{
    static const char *const kNames[8] = {
        "L1,L2C,L5,L1C-P,L2P",       /**< GPS (Obs2FreId FreList) */
        "L1,L5",                     /**< SBAS */
        "G1,G2",                     /**< GLONASS */
        "E1,E5A,E5B,E5(A+B),E6",     /**< Galileo */
        "L1,L2,L5,L1C(P),L6,L1C/B",  /**< QZSS */
        "B1,B2,B3,B1C,B2a,B2b",      /**< BDS */
        "L1L,L1H,L2L,L2H",           /**< XW1 */
        "L5,S",                      /**< NavIC (no Sys letter) */
    };
    return QString::fromLatin1(kNames[systemIndex]);
}

QString systemLabel(int systemIndex)
{
    static const char *const kLabels[8] = {
        "GPS", "SBAS", "GLONASS", "Galileo", "QZSS", "BDS", "XW1", "NavIC",
    };
    return QString::fromLatin1(kLabels[systemIndex]);
}

QChar systemLetter(int systemIndex)
{
    return QLatin1Char(kSysLetters[systemIndex]);
}

QString joinDoubles(const QVector<double> &values)
{
    QStringList items;
    for (double v : values) items << num(v);
    return items.join(QLatin1Char(','));
}

QString joinXYZ(const QVector<double> &values)
{
    QStringList items;
    for (double v : values) items << numXYZ(v);
    return items.join(QLatin1Char(','));
}

QString joinInts(const QVector<int> &values)
{
    QStringList items;
    for (int v : values) items << QString::number(v);
    return items.join(QLatin1Char(','));
}

} // namespace QCConfigFileModel
