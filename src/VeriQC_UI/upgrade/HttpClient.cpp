#include "HttpClient.h"

#include <QDebug>
#include <QFile>
#include <QHash>
#include <QUrlQuery>
#include <QHttpPart>
#include <QHttpMultiPart>

/*-----------------------------------------------------------------------------|
|                              HttpClientPrivate                              |
|----------------------------------------------------------------------------*/

enum class HttpClientRequestMethod {
    GET, POST, PUT, DELETE, UPLOAD
};


class HttpClientPrivateCache {
public:
    std::function<void(const QString&)>      successHandler = nullptr;
    std::function<void(const QString&, int)>    failHandler = nullptr;
    std::function<void()>                    completeHandler = nullptr;
    std::function<void(const QByteArray&)>downloadProgressHandler = nullptr;
    bool debug = false;
    bool internal = false;
    QString charset;
    QNetworkAccessManager* manager = nullptr;
};


class HttpClientPrivate {
    friend class HttpClient;

    HttpClientPrivate(const QString& url);
    ~HttpClientPrivate();
    void stop1();

    HttpClientPrivateCache cache();

    QNetworkAccessManager* getManager();

    static QNetworkRequest createRequest(HttpClientPrivate* d, HttpClientRequestMethod method);

    static void executeQuery(HttpClientPrivate* d, HttpClientRequestMethod method);

    static void upload(HttpClientPrivate* d, const QStringList& paths, const QByteArray& data);

    static void download(HttpClientPrivate* d, const QString& savePath);

    static void download(HttpClientPrivate* d, std::function<void(const QByteArray&)> readyRead);

    static QString readReply(QNetworkReply* reply, const QString& charset = "UTF-8");

    static void handleFinish(HttpClientPrivateCache cache, QNetworkReply* reply, const QString& successMessage, const QString& failMessage);

    /////////////////////////////////////////////////// Member Variables //////////////////////////////////////////////
    QString   url;                            // Request URL
    QString   json;                           // Request parameters in JSON format
    QUrlQuery params;                         // Request parameters in Form format
    QString   charset = "UTF-8";              // Character set for response
    QHash<QString, QString> headers;          // Request headers
    QNetworkAccessManager* manager = nullptr; // QNetworkAccessManager instance for HTTP requests
    bool useJson = false;                    // true: use JSON format for parameters, false: use Form format
    bool debug = false;                       // true: enable debug output (URL and parameters)
    bool internal = true;                     // Whether to use internally created manager

    std::function<void(const QString&)>   successHandler = nullptr; // Success callback (response content)
    std::function<void(const QString&, int)> failHandler = nullptr; // Failure callback (error message and HTTP status code)
    std::function<void()>                 completeHandler = nullptr; // Completion callback (no parameters)
    std::function<void(const QByteArray&)>downloadProgressHandler = nullptr; // Download progress callback (received data chunk)
};

HttpClientPrivate::HttpClientPrivate(const QString& url) : url(url) { }

HttpClientPrivate::~HttpClientPrivate() {
    manager = nullptr;
    successHandler = nullptr;
    failHandler = nullptr;
    completeHandler = nullptr;
}

void HttpClientPrivate::stop1()
{
    manager->deleteLater();
}

HttpClientPrivateCache HttpClientPrivate::cache() {
    HttpClientPrivateCache cache;

    cache.successHandler = successHandler;
    cache.failHandler = failHandler;
    cache.downloadProgressHandler = downloadProgressHandler;
    cache.completeHandler = completeHandler;
    cache.debug = debug;
    cache.internal = internal;
    cache.charset = charset;
    cache.manager = getManager();

    return cache;
}

void HttpClientPrivate::executeQuery(HttpClientPrivate* d, HttpClientRequestMethod method) {

    HttpClientPrivateCache cache = d->cache();

    QNetworkRequest request = HttpClientPrivate::createRequest(d, method);
    QNetworkReply* reply = nullptr;

    switch (method) {
    case HttpClientRequestMethod::GET:
        reply = cache.manager->get(request);
        break;
    case HttpClientRequestMethod::POST:
        reply = cache.manager->post(request, d->useJson ? d->json.toUtf8() : d->params.toString(QUrl::FullyEncoded).toUtf8());
        break;
    case HttpClientRequestMethod::PUT:
        reply = cache.manager->put(request, d->useJson ? d->json.toUtf8() : d->params.toString(QUrl::FullyEncoded).toUtf8());
        break;
    case HttpClientRequestMethod::DELETE:
        reply = cache.manager->deleteResource(request);
        break;
    default:
        break;
    }

    QObject::connect(reply, &QNetworkReply::finished, [=] {
        QString successMessage = HttpClientPrivate::readReply(reply, cache.charset.toUtf8());
        QString failMessage = reply->errorString();
        HttpClientPrivate::handleFinish(cache, reply, successMessage, failMessage);
        });
}

void HttpClientPrivate::download(HttpClientPrivate* d, const QString& savePath) {
    QFile* file = new QFile(savePath);

    if (!file->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file->close();
        file->deleteLater();

        if (d->debug) {
            qDebug().noquote() << QString("[Error] Failed to open file: %1").arg(savePath);
        }

        if (nullptr != d->failHandler) {
            d->failHandler(QString("[Error] Failed to open file: %1").arg(savePath), -1);
        }

        return;
    }

    std::function<void()> userCompleteHandler = d->completeHandler;
    std::function<void(const QByteArray&)>downloadProgressHandler = d->downloadProgressHandler;
    std::function<void()> injectedCompleteHandler = [=]() {
        file->flush();
        file->close();
        file->deleteLater();

        if (nullptr != userCompleteHandler) {
            userCompleteHandler();
        }
    };
    
    d->completeHandler = injectedCompleteHandler;

    HttpClientPrivate::download(d, [=](const QByteArray& data) {
        file->write(data); 
        downloadProgressHandler(data);
        });
}

void HttpClientPrivate::download(HttpClientPrivate* d, std::function<void(const QByteArray&)> readyRead) {
    HttpClientPrivateCache cache = d->cache();

    QNetworkRequest request = HttpClientPrivate::createRequest(d, HttpClientRequestMethod::GET);
    QNetworkReply* reply = cache.manager->get(request);

    QObject::connect(reply, &QNetworkReply::readyRead, [=] {
        readyRead(reply->readAll());
        });

    QObject::connect(reply, &QNetworkReply::finished, [=] {
        QString successMessage = "Download Done"; 
        QString failMessage = reply->errorString();
        HttpClientPrivate::handleFinish(cache, reply, successMessage, failMessage);
        });
}

void HttpClientPrivate::upload(HttpClientPrivate* d, const QStringList& paths, const QByteArray& data) {
    HttpClientPrivateCache cache = d->cache();

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QList<QPair<QString, QString> > paramItems = d->params.queryItems();
    for (int i = 0; i < paramItems.size(); ++i) {
        QString name = paramItems.at(i).first;
        QString value = paramItems.at(i).second;

        QHttpPart textPart;
        textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data; name=\"%1\"").arg(name));
        textPart.setBody(value.toUtf8());
        multiPart->append(textPart);
    }

    if (paths.size() > 0) {
        QString inputName = paths.size() == 1 ? "file" : "files";

        for (const QString& path : paths) {
            if (path.isEmpty()) {
                continue;
            }

            // We cannot delete the file now, so delete it with the multiPart
            QFile* file = new QFile(path, multiPart);

            if (!file->open(QIODevice::ReadOnly)) {
                QString failMessage = QString("Failed to open file[%2]: %1").arg(path).arg(file->errorString());

                if (cache.debug) {
                    qDebug().noquote() << failMessage;
                }

                if (nullptr != cache.failHandler) {
                    cache.failHandler(failMessage, -1);
                }

                multiPart->deleteLater();
                return;
            }

            QString disposition = QString("form-data; name=\"%1\"; filename=\"%2\"").arg(inputName).arg(file->fileName());
            QHttpPart filePart;
            filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(disposition));
            filePart.setBodyDevice(file);
            multiPart->append(filePart);
        }
    }
    else {
        QString   disposition = QString("form-data; name=\"file\"; filename=\"no-name\"");
        QHttpPart dataPart;
        dataPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(disposition));
        dataPart.setBody(data);
        multiPart->append(dataPart);
    }

    QNetworkRequest request = HttpClientPrivate::createRequest(d, HttpClientRequestMethod::UPLOAD);
    QNetworkReply* reply = cache.manager->post(request, multiPart);
    QObject::connect(reply, &QNetworkReply::finished, [=] {
        multiPart->deleteLater(); 

        QString successMessage = HttpClientPrivate::readReply(reply, cache.charset);
        QString failMessage = reply->errorString();
        HttpClientPrivate::handleFinish(cache, reply, successMessage, failMessage);
        });
}

QNetworkAccessManager* HttpClientPrivate::getManager() {
    return internal ? new QNetworkAccessManager() : manager;
}

QNetworkRequest HttpClientPrivate::createRequest(HttpClientPrivate* d, HttpClientRequestMethod method) {

    bool get = method == HttpClientRequestMethod::GET;
    bool upload = method == HttpClientRequestMethod::UPLOAD;
    bool withForm = !get && !upload && !d->useJson;
    bool withJson = !get && !upload && d->useJson;

    if (get && !d->params.isEmpty()) {
        d->url += "?" + d->params.toString(QUrl::FullyEncoded);
    }

    if (d->debug) {
        qDebug().noquote() << "[Url]" << d->url;

        if (withJson) {
            qDebug().noquote() << "[Param]" << d->json;
        }
        else if (withForm || upload) {
            QList<QPair<QString, QString> > paramItems = d->params.queryItems();
            QString buffer; 

            for (int i = 0; i < paramItems.size(); ++i) {
                QString name = paramItems.at(i).first;
                QString value = paramItems.at(i).second;

                if (0 == i) {
                    buffer += QString("[Param] %1=%2\n").arg(name).arg(value);
                }
                else {
                    buffer += QString("       %1=%2\n").arg(name).arg(value);
                }
            }

            if (!buffer.isEmpty()) {
                qDebug().noquote() << buffer;
            }
        }
    }

    if (withForm) {
        d->headers["Content-Type"] = "application/x-www-form-urlencoded";
    }
    else if (withJson) {
        d->headers["Content-Type"] = "application/json; charset=utf-8";
    }

    QNetworkRequest request(QUrl(d->url));
    for (auto i = d->headers.cbegin(); i != d->headers.cend(); ++i) {
        request.setRawHeader(i.key().toUtf8(), i.value().toUtf8());
    }

    return request;
}

QString HttpClientPrivate::readReply(QNetworkReply* reply, const QString& charset) {
    QTextStream in(reply);
    QString result;
    in.setCodec(charset.toUtf8());

    while (!in.atEnd()) {
        result += in.readLine();
    }

    return result;
}

void HttpClientPrivate::handleFinish(HttpClientPrivateCache cache, QNetworkReply* reply, const QString& successMessage, const QString& failMessage) {

    if (reply->error() == QNetworkReply::NoError) {
        if (cache.debug) {
            qDebug().noquote() << QString("[Finish] Success: %1").arg(successMessage);
        }

        if (nullptr != cache.successHandler) {
            cache.successHandler(successMessage);
        }
    }
    else {
        if (cache.debug) {
            qDebug().noquote() << QString("[Finish] Failure: %1").arg(failMessage);
        }

        if (nullptr != cache.failHandler) {
            cache.failHandler(failMessage, reply->error());
        }
    }

    if (nullptr != cache.completeHandler) {
        cache.completeHandler();
    }

    if (nullptr != reply) {
        reply->deleteLater();
    }

    if (cache.internal && nullptr != cache.manager) {
        cache.manager->deleteLater();
    }
}

/*-----------------------------------------------------------------------------|
|                                 HttpClient                                  |
|----------------------------------------------------------------------------*/

// Note: In asynchronous requests, the HttpClientPrivate member variable 'd' of HttpClient has been destroyed. 
// Cache relevant variables as stack objects first and use [=] for value capture
HttpClient::HttpClient(const QString& url) : d(new HttpClientPrivate(url)) { }

HttpClient::~HttpClient() {
    delete d;
}

void HttpClient::stop2()
{
    d->stop1();
}

HttpClient& HttpClient::manager(QNetworkAccessManager* manager) {
    d->manager = manager;
    d->internal = (nullptr == manager);

    return *this;
}

HttpClient& HttpClient::debug(bool debug) {
    d->debug = debug;

    return *this;
}

HttpClient& HttpClient::param(const QString& name, const QVariant& value) {
    d->params.addQueryItem(name, value.toString());

    return *this;
}

HttpClient& HttpClient::params(const QMap<QString, QVariant>& ps) {
    for (auto iter = ps.cbegin(); iter != ps.cend(); ++iter) {
        d->params.addQueryItem(iter.key(), iter.value().toString());
    }

    return *this;
}

HttpClient& HttpClient::json(const QString& json) {
    d->json = json;
    d->useJson = true;

    return *this;
}

HttpClient& HttpClient::header(const QString& name, const QString& value) {
    d->headers[name] = value;

    return *this;
}

HttpClient& HttpClient::headers(const QMap<QString, QString> nameValues) {
    for (auto i = nameValues.cbegin(); i != nameValues.cend(); ++i) {
        d->headers[i.key()] = i.value();
    }

    return *this;
}

HttpClient& HttpClient::success(std::function<void(const QString&)> successHandler) {
    d->successHandler = successHandler;

    return *this;
}
HttpClient& HttpClient::downloadProgress(std::function<void(const QByteArray&)> downloadProgressHandler) {
    d->downloadProgressHandler = downloadProgressHandler; 
    return *this;
}

HttpClient& HttpClient::fail(std::function<void(const QString&, int)> failHandler) {
    d->failHandler = failHandler;

    return *this;
}

HttpClient& HttpClient::complete(std::function<void()> completeHandler) {
    d->completeHandler = completeHandler;

    return *this;
}

HttpClient& HttpClient::charset(const QString& cs) {
    d->charset = cs;

    return *this;
}

void HttpClient::get() {
    HttpClientPrivate::executeQuery(d, HttpClientRequestMethod::GET);
}

void HttpClient::post() {
    HttpClientPrivate::executeQuery(d, HttpClientRequestMethod::POST);
}

void HttpClient::put() {
    HttpClientPrivate::executeQuery(d, HttpClientRequestMethod::PUT);
}

void HttpClient::remove() {
    HttpClientPrivate::executeQuery(d, HttpClientRequestMethod::DELETE);
}

void HttpClient::download(const QString& savePath) {
    HttpClientPrivate::download(d, savePath);
}

void HttpClient::upload(const QString& path) {
    QStringList paths = { path };
    HttpClientPrivate::upload(d, paths, QByteArray());
}

void HttpClient::upload(const QByteArray& data) {
    HttpClientPrivate::upload(d, QStringList(), data);
}

void HttpClient::upload(const QStringList& paths) {
    HttpClientPrivate::upload(d, paths, QByteArray());
}