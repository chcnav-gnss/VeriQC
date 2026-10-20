/**********************************************************************//**
 *
 * @file PathUtils.h
 * @author CHC
 * @date 2026-07-20
 * @brief Encode QString file/dir paths into byte sequences that the
 *        C core library's fopen() can decode reliably on Windows.
 *
 * The C core library (src/VeriQC) opens every file with fopen(char*),
 * which on MSVC CRT decodes path bytes using the system ANSI codepage
 * (GBK on Simplified Chinese locale). Non-ASCII paths therefore require
 * the UI layer to encode paths into bytes fopen can map back to the file.
 *
 * Strategy on Windows: prefer the 8.3 short path name returned by
 * GetShortPathNameW (locale-independent ASCII, and survives the
 * QString(const char*) UTF-8 round-trip used elsewhere in the UI),
 * falling back to toLocal8Bit() when the API fails (non-existent path,
 * 8.3 disabled volume, UNC). On other platforms toLocal8Bit() is used.
 *
 * Applies to every UI->C path field: RoverFile / BaseFile / RawFile /
 * OutDir / config file path / executable path.
 *
 **************************************************************************/
#ifndef PATHUTILS_H
#define PATHUTILS_H

#include <QByteArray>
#include <QString>
#include <cstring>    /**< std::memcpy */

/// Path encoding utilities for the UI->C boundary (see file header).
class PathUtils
{
public:
    /**
     * @brief Encode a file path to bytes safe for the C layer's fopen().
     * @param path  Absolute file path (UTF-16). Empty input yields empty output.
     * @return QByteArray for memcpy into CHCQC_GNSSCFG_T / CHCQC_RINEXCFG_T
     *         char[] fields, or for fopen() directly.
     */
    static QByteArray toLocalFilePathBytes(const QString &path);

    /**
     * @brief Encode a directory path to bytes safe for the C layer; creates
     *        the directory via QDir::mkpath() first (using the original
     *        QString to avoid the char[]->QString UTF-8 misdecode).
     * @param dir  Absolute directory path; created if missing. Trailing
     *             separator appended if absent.
     * @return Non-empty QByteArray on success; empty if mkpath fails.
     */
    static QByteArray toLocalDirPathBytes(const QString &dir);

    /**
     * @brief Encode @p path via toLocalFilePathBytes() and memcpy it into a
     *        fixed-size C char[] destination, NUL-terminating the result.
     *
     * Consolidates the encode-then-truncated-memcpy pattern repeated at every
     * UI->C path field (RoverFile / BaseFile / RinexEphFile / RawFile / OutDir).
     * Copies at most @p dstSize - 1 bytes and always writes a terminating '\0'
     * (the destination is zeroed by the C-layer InitXxxCfg() helpers, but this
     * is defensive against future reuse on uninitialised buffers). Empty @p path
     * leaves the destination untouched apart from the leading NUL.
     *
     * @tparam N    char[] field size (deduced); must be > 0.
     * @param dst   C-layer destination char[] (e.g. apiConfig.RoverFile).
     * @param path  Source path as a QString; encoded by toLocalFilePathBytes().
     * @return Number of bytes copied (excluding the NUL terminator).
     */
    template <int N>
    static int copyFilePathBytes(char (&dst)[N], const QString &path)
    {
        return copyEncodedPathBytes(dst, toLocalFilePathBytes(path));
    }

    /**
     * @brief Encode @p dir via toLocalDirPathBytes() (creates the directory and
     *        appends a trailing separator, which the C layer's output-path
     *        concatenation requires) and copy it into a fixed-size C char[]
     *        destination, NUL-terminating the result.
     *
     * Directory-flavoured counterpart of copyFilePathBytes(); use it for the
     * OutDir fields so a shorter command-line override never leaves bytes of a
     * previous (config-file) value behind. An empty result (mkpath failure)
     * writes just a NUL so the C layer falls back to the input file directory.
     *
     * @tparam N    char[] field size (deduced); must be > 0.
     * @param dst   C-layer destination char[] (e.g. apiConfig.OutDir).
     * @param dir   Source directory as a QString; encoded by toLocalDirPathBytes().
     * @return Number of bytes copied (excluding the NUL terminator).
     */
    template <int N>
    static int copyDirPathBytes(char (&dst)[N], const QString &dir)
    {
        return copyEncodedPathBytes(dst, toLocalDirPathBytes(dir));
    }

private:
    /// Windows short-path conversion with toLocal8Bit() fallback (see header).
    static QByteArray toSafeLocalPath(const QString &path);

    /**
     * @brief Shared copy core of copyFilePathBytes() / copyDirPathBytes():
     *        clamp the encoded bytes to the field size, memcpy, NUL-terminate.
     *
     * @tparam N    char[] field size (deduced); must be > 0.
     * @param dst   C-layer destination char[].
     * @param bytes Path bytes already encoded by toLocalFilePathBytes() /
     *              toLocalDirPathBytes().
     * @return Number of bytes copied (excluding the NUL terminator).
     */
    template <int N>
    static int copyEncodedPathBytes(char (&dst)[N], const QByteArray &bytes)
    {
        /** QByteArray::size() is qsizetype in Qt6 / int in Qt5; clamp to N-1
         *  so the destination never overflows (max N-1 payload + NUL). */
        const int srcLen = static_cast<int>(bytes.size());
        const int copyLen = (srcLen < N - 1) ? srcLen : (N - 1);
        if (copyLen > 0)
        {
            std::memcpy(dst, bytes.constData(), static_cast<size_t>(copyLen));
        }
        dst[copyLen] = '\0';
        return copyLen;
    }
};

#endif // PATHUTILS_H
