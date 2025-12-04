#include "httpmanager.h"
#include "hash_processor.h"

#include <QNetworkRequest>
#include <QJsonDocument>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QCryptographicHash>

HttpManager::HttpManager(QObject* parent)
    : QObject(parent)
    , m_baseUrl("http://localhost:8080/api")
{
    connect(&m_manager, &QNetworkAccessManager::finished,
        this, &HttpManager::onReplyFinished);
}

void HttpManager::serBaseUrl(const QString& url)
{
    m_baseUrl = url;
}

// POST /auth/register
void HttpManager::registerUser(const QString& username, const QString& password)
{
    QUrl url(m_baseUrl + "/auth/register");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    

    QJsonObject body;
    body["username"] = username;
    // 密码进行sha256加密
    body["passwordHash"] = HashEncrypt(password);

    QNetworkReply* reply =
        m_manager.post(request, QJsonDocument(body).toJson());
    reply->setProperty("requestType", "register");
}

// POST /auth/login
void HttpManager::login(const QString& username, const QString& password)
{
    QUrl url(m_baseUrl + "/auth/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["username"] = username;
    body["passwordHash"] = HashEncrypt(password);

    QNetworkReply* reply =
        m_manager.post(request, QJsonDocument(body).toJson());
    reply->setProperty("requestType", "login");
}

// POST /auth/logout
void HttpManager::logout(const QString& token)
{
    QUrl url(m_baseUrl + "/auth/logout");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Auth-Token", token.toUtf8());

    QNetworkReply* reply = m_manager.post(request, QByteArray());
    reply->setProperty("requestType", "logout");
}

void HttpManager::onReplyFinished(QNetworkReply* reply)
{
    const QString reqType = reply->property("requestType").toString();

    if (reply->error() != QNetworkReply::NoError) {
        emit networkError(reply->errorString());
        reply->deleteLater();
        return;
    }

    const QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        emit networkError(QStringLiteral("无效的 JSON 响应"));
        reply->deleteLater();
        return;
    }

    QJsonObject obj = doc.object();

    if (reqType == "login") {
        handleLoginResponse(obj);
    }
    else if (reqType == "register") {
        handleRegisterResponse(obj);
    }
    else if (reqType == "logout") {
        handleLogoutResponse(obj);
    }

    reply->deleteLater();
}

void HttpManager::handleLoginResponse(const QJsonObject& obj)
{
    const int code = obj.value("code").toInt(1);
    const QString msg = obj.value("message").toString();

    if (code != 0) {
        emit loginResult(false, msg, QString(), QJsonObject());
        return;
    }

    const QString token = obj.value("token").toString();
    const QJsonObject noteStruct =
        obj.value("note_structure").toObject(); // 允许为空

    // 可选：把 note_structure 保存成 json 文件
    if (!noteStruct.isEmpty()) {
        saveNoteStructureToFile(noteStruct);
    }

    emit loginResult(true, msg, token, noteStruct);
}

void HttpManager::handleRegisterResponse(const QJsonObject& obj)
{
    const int code = obj.value("code").toInt(1);
    const QString msg = obj.value("message").toString();
    emit registerResult(code == 0, msg);
}

void HttpManager::handleLogoutResponse(const QJsonObject& obj)
{
    const int code = obj.value("code").toInt(1);
    const QString msg = obj.value("message").toString();
    emit logoutResult(code == 0, msg);
}

// 简单把 note_structure 写到用户文档目录
void HttpManager::saveNoteStructureToFile(const QJsonObject& noteStruct)
{
    const QString dirPath =
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
        + "/NoteClient";
    QDir().mkpath(dirPath);

    QFile file(dirPath + "/note_structure.json");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }

    QJsonDocument doc(noteStruct);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
}
