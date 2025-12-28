#include "CZipExtractor.h"
#include <QDir>
#include <QFileInfo>
#include <QList>

#ifdef _WIN32
#include <QtGui/5.12.12/QtGui/private/qzipreader_p.h>
#else
#include <QtGui/5.12.8/QtGui/private/qzipreader_p.h>
#endif // _WIN32


CZipExtractor::CZipExtractor(QObject* parent)
    :QThread(parent)
{

}

void CZipExtractor::StartUnzip(const QString& ZipFile, const QString& ExtractDir)
{
    m_ZipFile = ZipFile;
    m_ExtractDir = ExtractDir;
    this->start();
}

bool CZipExtractor::extractAll(QZipReader &ZipReader, const QString& destinationDir)
{
    int TotalFiles = ZipReader.count();
    int ExtractedFiles = 0;
    QDir baseDir(destinationDir);

    // create directories first
    const QVector<QZipReader::FileInfo> allFiles = ZipReader.fileInfoList();
    bool foundDirs = false;
    bool hasDirs = false;
    for (const QZipReader::FileInfo& fi : allFiles) {
        const QString absPath = destinationDir + QDir::separator() + fi.filePath;
        if (fi.isDir) {
            foundDirs = true;
            if (!baseDir.mkpath(fi.filePath))
                return false;
            if (!QFile::setPermissions(absPath, fi.permissions))
                return false;
        }
        else if (!hasDirs && fi.filePath.contains("/")) {
            // filePath does not have leading or trailing '/', so if we find
            // one, than the file path contains directories.
            hasDirs = true;
        }
    }

    // Some zip archives can be broken in the sense that they do not report
    // separate entries for directories, only for files. In this case we
    // need to recreate directory structure based on the file paths.
    if (hasDirs && !foundDirs) {
        for (const QZipReader::FileInfo& fi : allFiles) {
            if (!fi.filePath.contains("/"))
                continue;
            const auto dirPath = fi.filePath.left(fi.filePath.lastIndexOf("/"));
            if (!baseDir.mkpath(dirPath))
                return false;
            // We will leave the directory permissions default in this case,
            // because setting dir permissions based on file is incorrect
        }
    }

    for (const QZipReader::FileInfo& fi : allFiles) {
        const QString absPath = destinationDir + QDir::separator() + fi.filePath;
        if (fi.isFile) {
            QFile f(absPath);
            if (!f.open(QIODevice::WriteOnly))
                return false;
            f.write(ZipReader.fileData(fi.filePath));
            f.setPermissions(fi.permissions);
            f.close();
        }
        ExtractedFiles++;
        int progress = static_cast<int>((ExtractedFiles * 100) / TotalFiles);
        emit ProgressUpdateSignal(progress);
    }

    // set up symlinks
    for (const QZipReader::FileInfo& fi : allFiles) {
        const QString absPath = destinationDir + QDir::separator() + fi.filePath;
        if (fi.isSymLink) {
            QString destination = QFile::decodeName(ZipReader.fileData(fi.filePath));
            if (destination.isEmpty())
                return false;
            QFileInfo linkFi(absPath);
            if (!QFile::exists(linkFi.absolutePath()))
                QDir::root().mkpath(linkFi.absolutePath());
            if (!QFile::link(destination, absPath))
                return false;
            /* cannot change permission of links
            if (!QFile::setPermissions(absPath, fi.permissions))
                return false;
            */
        }
    }

    return true;
}

void CZipExtractor::run()
{
    QZipReader ZipReader(m_ZipFile);
    if (ZipReader.isReadable())
    {
        if (this->extractAll(ZipReader, m_ExtractDir))
        {
            emit ExtractFinished(true);
            return;
        }
    }
    emit ExtractFinished(false);
}
