#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H 
#include <functional>
#include <QMap>
#include <QVariant>
#include <QStringList>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager> 
class  HttpClientPrivate; 
/**
* A simple HTTP client wrapper for QNetworkAccessManager, simplifying operations 
* like GET, POST, PUT, DELETE, upload and download.
* 
* Before executing requests, configure parameters and callbacks:
*     1. Use header() to set request headers
*     2. Use param() to set form parameters (also for GET query parameters)
*     3. Use json() to set request body with JSON format (Content-Type: application/json)
*     4. Use success() to register success callback
*     5. Use fail() to register failure callback
*     6. Use complete() to register completion callback
* 
* Call get(), post(), put(), remove(), download(), upload() to execute requests.
*/
class  HttpClient {
public:
    HttpClient(const QString& url);
    ~HttpClient();
    void stop2();

    /**
    * @brief Share QNetworkAccessManager between requests to save thread resources.
    *        If not set, HttpClient will create and auto-delete its own manager.
    * 
    * @param manager The QNetworkAccessManager instance to be shared
    * @return HttpClient reference for chaining
    */
    HttpClient& manager(QNetworkAccessManager* manager);

    HttpClient& debug(bool debug);

    HttpClient& param(const QString& name, const QVariant& value);

    HttpClient& params(const QMap<QString, QVariant>& ps);

    HttpClient& json(const QString& json);

    HttpClient& header(const QString& name, const QString& value);

    HttpClient& headers(const QMap<QString, QString> nameValues);

    HttpClient& success(std::function<void(const QString&)> successHandler);
    HttpClient& downloadProgress(std::function<void(const QByteArray&)> downloadProgressHandler);

    HttpClient& fail(std::function<void(const QString&, int)> failHandler);

    HttpClient& complete(std::function<void()> completeHandler);

    HttpClient& charset(const QString& cs);

    void get();

    void post();

    void put();

    void remove();

    void download(const QString& savePath);

    void upload(const QString& path);

    void upload(const QByteArray& data);

    void upload(const QStringList& paths);

private:
    HttpClientPrivate* d;
};

#endif // HTTPCLIENT_H