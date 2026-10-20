/**********************************************************************//**
 *
 * @file PathUtils.cpp
 * @author CHC
 * @date 2026-07-20
 * @brief Implementation of path encoding utilities (see PathUtils.h).
 *
 **************************************************************************/
#include "PathUtils.h"
#include <QDir>
#include <QDebug>
#include <vector>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

/**
 * @brief Convert a path to bytes safe for the C layer's fopen().
 *
 * Windows: prefer the 8.3 short path (locale-independent ASCII), falling
 * back to toLocal8Bit() when GetShortPathNameW fails (non-existent path,
 * 8.3 disabled volume, UNC). Non-Windows: toLocal8Bit() directly.
 *
 * @note The fallback uses toLocal8Bit() (NOT toLatin1()). On a Simplified
 *       Chinese locale this yields GBK bytes that fopen decodes correctly;
 *       toLatin1() would replace non-ASCII bytes with '?', corrupting the
 *       path. This was the root cause of the "Failed to open observation
 *       file" error on volumes with 8.3 disabled or non-ASCII short names.
 */
QByteArray PathUtils::toSafeLocalPath(const QString &path)
{
#ifdef Q_OS_WIN
    /** Fast path: pure-ASCII paths (exe dir, English-locale data) are
     *  byte-identical under toLocal8Bit(), so skip the GetShortPathNameW
     *  syscall. Only non-ASCII paths need the 8.3 short-name lookup. */
    bool needsShort = false;
    for (const QChar c : path)
    {
        if (c.unicode() > 0x7F)
        {
            needsShort = true;
            break;
        }
    }
    if (!needsShort)
    {
        return path.toLocal8Bit();
    }
    wchar_t stackBuf[MAX_PATH];
    /** Return value excludes the null terminator on success. */
    DWORD result = GetShortPathNameW(
        reinterpret_cast<const wchar_t *>(path.utf16()),
        stackBuf, MAX_PATH);
    if (result == 0)
    {
        /** GetShortPathNameW failure usually means 8.3 short names are
         *  disabled on this volume, or the path does not exist. */
        qWarning("PathUtils: GetShortPathNameW failed err=%lu path=%s",
                 static_cast<unsigned long>(GetLastError()), qPrintable(path));
        return path.toLocal8Bit();
    }
    if (result < MAX_PATH)
    {
        /** Common case: short path fits in the stack buffer. toLocal8Bit()
         *  preserves ASCII unchanged and encodes residual non-ASCII under
         *  the system locale. */
        return QString::fromWCharArray(stackBuf, result).toLocal8Bit();
    }
    /** Path exceeds MAX_PATH (rare); retry with a heap buffer sized to the
     *  length returned by the first call (includes the null terminator). */
    std::vector<wchar_t> heapBuf(result);
    DWORD result2 = GetShortPathNameW(
        reinterpret_cast<const wchar_t *>(path.utf16()),
        heapBuf.data(), result);
    if (result2 == 0 || result2 >= result)
    {
        return path.toLocal8Bit();
    }
    return QString::fromWCharArray(heapBuf.data(), result2).toLocal8Bit();
#else
    /** Non-Windows: no ANSI codepage constraint, use locale encoding directly. */
    return path.toLocal8Bit();
#endif
}

QByteArray PathUtils::toLocalFilePathBytes(const QString &path)
{
    /** Callers guarantee non-empty input (exe path, entryInfoList outputs,
     *  configFilePath-after-isEmpty guard, rawFilePath at generate time).
     *  toSafeLocalPath returns an empty QByteArray for empty input anyway. */
    return toSafeLocalPath(path);
}

QByteArray PathUtils::toLocalDirPathBytes(const QString &dir)
{
    if (dir.isEmpty())
    {
        return QByteArray();
    }
    /** Normalize: ensure a trailing separator so C-layer path concatenation
     *  (sprintf "%s%s", outDir, name) yields a well-formed path. */
    QString normalized = dir;
    if (!normalized.endsWith('/') && !normalized.endsWith('\\'))
    {
        normalized += QDir::separator();
    }
    /** Create the directory using the original QString to avoid the
     *  char[]->QString UTF-8 misdecode that would corrupt GBK bytes.
     *  mkpath is idempotent for existing directories. */
    if (!QDir().mkpath(normalized))
    {
        qWarning("PathUtils: mkpath failed path=%s", qPrintable(normalized));
        return QByteArray();
    }
    return toSafeLocalPath(normalized);
}
