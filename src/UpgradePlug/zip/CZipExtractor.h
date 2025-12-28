#ifndef CZIPEXTRACTOR_H
#define CZIPEXTRACTOR_H

#include <QThread>
#include <QString>

class QZipReader;
class CZipExtractor : public QThread
{
    Q_OBJECT
public:
    CZipExtractor(QObject* parent = nullptr);
    void StartUnzip(const QString& ZipFile, const QString& ExtractDir);
signals:
    void ProgressUpdateSignal(int Precent);
    void ExtractFinished(bool bSuccess);
protected:
    virtual void run() override;
private:
    bool extractAll(QZipReader& ZipReader, const QString& destinationDir);

    QString m_ZipFile;
    QString m_ExtractDir;
};

#endif // CZIPEXTRACTOR_H
