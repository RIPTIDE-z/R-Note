#include "httpmanager.h"
#include <QNetworkRequest>
#include <QDebug>

HttpManager::HttpManager(QObject* parent)
    : QObject(parent)
    , manager(new QNetworkAccessManager(this))
    , baseUrl("http://localhost:8080/api/students")
{
    connect(manager, &QNetworkAccessManager::finished,
        this, &HttpManager::onReplyFinished);
}

HttpManager::~HttpManager()
{
    delete manager;
}

void HttpManager::getAllStudents()
{
    sendRequest("GET", "");
}

void HttpManager::getStudentById(int id)
{
    sendRequest("GET", "/" + QString::number(id));
}

void HttpManager::addStudent(const QString& name, int age, const QString& className)
{
    QJsonObject data;
    data["name"] = name;
    data["age"] = age;
    data["className"] = className;
    sendRequest("POST", "/with-log", data);
}

void HttpManager::updateStudent(int id, const QString& name, int age, const QString& className)
{
    QJsonObject data;
    data["name"] = name;
    data["age"] = age;
    data["className"] = className;
    sendRequest("PUT", "/" + QString::number(id), data);
}

void HttpManager::deleteStudent(int id)
{
    sendRequest("DELETE", "/" + QString::number(id));
}

void HttpManager::sendRequest(const QString& method, const QString& endpoint,
    const QJsonObject& data)
{
    QUrl url(baseUrl + endpoint);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = nullptr;

    if (method == "GET") {
        reply = manager->get(request);
    }
    else if (method == "POST") {
        QJsonDocument doc(data);
        reply = manager->post(request, doc.toJson());
    }
    else if (method == "PUT") {
        QJsonDocument doc(data);
        reply = manager->put(request, doc.toJson());
    }
    else if (method == "DELETE") {
        reply = manager->deleteResource(request);
    }

    if (!reply) {
        emit requestError("请求创建失败");
    }
}

void HttpManager::onReplyFinished(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);

        if (doc.isObject()) {
            emit requestFinished(doc.object());
        }
        else {
            emit requestError("无效的 JSON 响应");
        }
    }
    else {
        emit requestError(reply->errorString());
    }

    reply->deleteLater();
}