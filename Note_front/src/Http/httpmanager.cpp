#include "httpmanager.h"
#include "hash_processor.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonParseError>

HttpManager::HttpManager(QObject* parent)
    : QObject(parent)
    , m_baseUrl("http://localhost:8080/api")   // 可以被 Config 覆盖
{
    connect(&m_manager, &QNetworkAccessManager::finished,
        this, &HttpManager::onReplyFinished);
}

void HttpManager::setBaseUrl(const QString& url)
{
    m_baseUrl = url;
}

// POST /auth/register
void HttpManager::registerUser(const QString& username,
    const QString& password)
{
    // 设置接口Url
    QUrl url(m_baseUrl + "/auth/register");
    QNetworkRequest request(url);
    // 设置请求头部，接受json
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 设置请求体内容
    QJsonObject body;
    body["username"] = username;
    body["passwordHash"] = HashEncrypt(password);

    // 创建响应且指定响应类型
    QNetworkReply* reply =
        m_manager.post(request, QJsonDocument(body).toJson());
    reply->setProperty("requestType", "register");
}

// POST /auth/login
void HttpManager::login(const QString& username,
    const QString& password)
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

// GET /note-structure
void HttpManager::fetchNoteStructure(const QString& token)
{
    QUrl url(m_baseUrl + "/note-structure");
    QNetworkRequest request(url);
    request.setRawHeader("Auth-Token", token.toUtf8());

    QNetworkReply* reply = m_manager.get(request);
    reply->setProperty("requestType", "fetchNoteStructure");
}

// PUT /note-structure
void HttpManager::updateNoteStructure(const QString& token,
    const QJsonObject& noteStruct)
{
    QUrl url(m_baseUrl + "/note-structure");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Auth-Token", token.toUtf8());

    QJsonDocument doc(noteStruct);
    QNetworkReply* reply = m_manager.put(request, doc.toJson());
    reply->setProperty("requestType", "updateNoteStructure");
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
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        emit networkError(
            QStringLiteral("无效的 JSON 响应: %1").arg(err.errorString()));
        reply->deleteLater();
        return;
    }

    QJsonObject obj = doc.object();

    // 根据响应类型调用对应的响应处理函数
    if (reqType == "login") {
        handleLoginResponse(obj);
    }
    else if (reqType == "register") {
        handleRegisterResponse(obj);
    }
    else if (reqType == "logout") {
        handleLogoutResponse(obj);
    }
    else if (reqType == "fetchNoteStructure") {
        handleFetchNoteStructureResponse(obj);
    }
    else if (reqType == "updateNoteStructure") {
        handleUpdateNoteStructureResponse(obj);
    }

    reply->deleteLater();
}

void HttpManager::handleLoginResponse(const QJsonObject& obj)
{
    // 获取响应体里的数据
    const int code = obj.value("code").toInt(1);
    const QString msg = obj.value("message").toString();

    // 1代表登录失败
    if (code == 1) {
        // 发出signal
        emit loginResult(false, msg, QString(), QJsonObject());
        return;
    }

    const QString token = obj.value("token").toString();
    const QJsonObject noteStruct =
        obj.value("note_structure").toObject();   // 允许为空

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

void HttpManager::handleFetchNoteStructureResponse(const QJsonObject& obj)
{
    const int code = obj.value("code").toInt(1);
    const QString msg = obj.value("message").toString();

    // 失败处理，返回空结构
    if (code == 1) {
        emit noteStructureFetched(false, msg, QJsonObject());
        return;
    }

    QJsonObject structureObj;

    // 后端使用 @JsonRawValue String noteStructure;
    // 返回的 noteStructure 在 JSON 里是一个嵌套对象
    // 直接拿对象就行
    const QJsonValue v = obj.value("noteStructure");

    if (v.isObject()) {
        // 标准情况：raw JSON 直接嵌套成对象
        structureObj = v.toObject();
    }
    // else if (v.isString()) {
    //     // 兼容一种情况：后端没加 @JsonRawValue，仍然是字符串，就走老逻辑
    //     const QString structureStr = v.toString();
    //     if (!structureStr.isEmpty()) {
    //         QJsonParseError err;
    //         QJsonDocument structDoc =
    //             QJsonDocument::fromJson(structureStr.toUtf8(), &err);
    //         if (err.error != QJsonParseError::NoError || !structDoc.isObject()) {
    //             emit noteStructureFetched(
    //                 false,
    //                 QStringLiteral("服务端返回的 noteStructure 不是合法 JSON: %1")
    //                 .arg(err.errorString()),
    //                 QJsonObject());
    //             return;
    //         }
    //         structureObj = structDoc.object();
    //     }
    // }
    else if (!v.isUndefined() && !v.isNull()) {
        // 字段存在但既不是 object 也不是 string，类型不对
        emit noteStructureFetched(
            false,
            QStringLiteral("服务端返回的 noteStructure 类型不正确"),
            QJsonObject());
        return;
    }

    // 不写本地文件，直接把 JSON 抛给上层，由 note_structure_manager 处理
    emit noteStructureFetched(true, msg, structureObj);
}

void HttpManager::handleUpdateNoteStructureResponse(const QJsonObject& obj)
{
    const int code = obj.value("code").toInt(1);
    const QString msg = obj.value("message").toString();

    emit noteStructureUpdated(code == 0, msg);
}

