#include "httpmanager.h"
#include "hash_processor.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonParseError>

HttpManager::HttpManager(QObject* parent)
    : QObject(parent)
    , m_baseUrl("http://127.0.0.1:8080/api")   // 可以被 Config 覆盖
{
    // 将onReplyFinished连接，请求结束会进入其中处理
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

    // post发出请求并把响应存入reply
    QNetworkReply* reply =
        m_manager.post(request, QJsonDocument(body).toJson());
    // 指定reply的类型，以便onReplyFinished辨别
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

// DEL /notes/{noteId}
void HttpManager::deleteNote(const QString& token, const int noteId)
{
}

// DEL /notes/{noteId}
void HttpManager::updateNote(const QString& token, const int noteId, const int code)
{
}

// GET /notes/{noteId}/{version}
void HttpManager::getNoteByVersion(const QString& token, const int noteId, const int version)
{
}

// GET /notes/{noteId}/histories
void HttpManager::getHistoryList(const QString& token, const int noteId)
{
}

// GET /note-structure
void HttpManager::fetchNoteStructure(const QString& token)
{
    QUrl url(m_baseUrl + "/note-structure");

    QNetworkRequest request(url);
    request.setRawHeader("Auth-Token", token.toUtf8());

    QNetworkReply* reply = m_manager.get(request);
    qDebug() << "已发送请求" << request.url();
    reply->setProperty("requestType", "fetchNoteStructure");

}

// PUT /note-structure
void HttpManager::updateNoteStructure(const QString& token,
    const QJsonObject& noteStruct)
{
    QUrl url(m_baseUrl + "/note-structure");
    qDebug() << "[Http] updateNoteStructure() baseUrl =" << m_baseUrl
        << ", url =" << url.toString()
        << ", token =" << token;

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Auth-Token", token.toUtf8());

    QJsonDocument doc(noteStruct);
    QNetworkReply* reply = m_manager.put(request, doc.toJson(QJsonDocument::Compact));
    reply->setProperty("requestType", "updateNoteStructure");

    connect(reply, &QNetworkReply::errorOccurred,
        this, [url](QNetworkReply::NetworkError code) {
            qDebug() << "[Http] updateNoteStructure error for" << url.toString()
                << ", code =" << code;
        });
}


void HttpManager::onReplyFinished(QNetworkReply* reply)
{
    const QString reqType = reply->property("requestType").toString();

    if (reply->error() != QNetworkReply::NoError) {
        emit networkError(reply->errorString());
        reply->deleteLater();
        return;
    }

    int statusCode = reply->attribute(
        QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QVariant reason = reply->attribute(
        QNetworkRequest::HttpReasonPhraseAttribute);

    const QByteArray data = reply->readAll();

    qDebug() << "[HttpManager] reply finished, type =" << reqType
        << ", error =" << reply->error()
        << ", status =" << statusCode
        << ", reason =" << reason.toString();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        emit networkError(
            QStringLiteral("无效的 JSON 响应: %1").arg(err.errorString()));
        reply->deleteLater();
        return;
    }


    QJsonObject obj = doc.object();

    // 根据响应类型调对应的响应处理函数
    if (reqType == "login") {
        handleLoginResponse(obj);
    }
    else if (reqType == "register") {
        handleRegisterResponse(obj);
    }
    else if (reqType == "logout") {
        handleLogoutResponse(obj);
    }
    else if (reqType == "deleteNode") {
        handleDeleteNoteResponse(obj);
    }
    else if (reqType == "updateNote") {
        handleUpdateNoteResponse(obj);
    }
    else if (reqType == "getNoteByVersion") {
        handleGetNoteByVersionResponse(obj);
    }
    else if (reqType == "getHostoryList") {
        handleGetHistoryListResponset(obj);
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

    // 使用信号将处理完成后的数据发出去，对应
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
    int code = obj.value("code").toInt(-1);
    QString msg = obj.value("message").toString();

    qDebug() << "正在处理返回的笔记结构";

    if (code != 0) {
        if (msg.isEmpty())
            msg = QStringLiteral("获取笔记结构失败");
        emit fetchNoteStructureResult(false, msg, QJsonObject{});
        return;
    }

    QJsonObject structureObj;

    // 后端字段名是 structure
    const QJsonValue v = obj.value("structure");
    qDebug() << "已拿取到QJson值, type =" << v.type();

    if (v.isString()) {
        const QString structureStr = v.toString();
        if (!structureStr.isEmpty()) {
            QJsonParseError err;
            QJsonDocument structDoc =
                QJsonDocument::fromJson(structureStr.toUtf8(), &err);
            if (err.error != QJsonParseError::NoError || !structDoc.isObject()) {
                qDebug() << "解析 structure JSON 失败:" << err.errorString();
                emit fetchNoteStructureResult(
                    false,
                    QStringLiteral("服务端返回的 structure 不是合法 JSON: %1")
                    .arg(err.errorString()),
                    QJsonObject{});
                return;
            }
            structureObj = structDoc.object();
            qDebug() << "已拿取到QJson对象";
        }
    }
    else if (v.isObject()) {
        // 如果以后你又加回 @JsonRawValue 变成嵌套对象，这里也兼容
        structureObj = v.toObject();
        qDebug() << "已拿取到QJson对象(嵌套 object)";
    }
    else if (!v.isNull() && !v.isUndefined()) {
        qDebug() << "处理出现错误: structure 类型不正确, type =" << v.type();
        emit fetchNoteStructureResult(
            false,
            QStringLiteral("服务端返回的 structure 类型不正确"),
            QJsonObject{});
        return;
    }

    if (msg.isEmpty())
        msg = QStringLiteral("成功获取笔记结构");

    // 成功时一定要 emit，把结构抛给上层（EditorWindow::onFetchResult）
    emit fetchNoteStructureResult(true, msg, structureObj);
}


void HttpManager::handleUpdateNoteStructureResponse(const QJsonObject& obj)
{
    const int code = obj.value("code").toInt(1);
    const QString msg = obj.value("message").toString();

    emit noteStructureUpdated(code == 0, msg);
}

void HttpManager::handleDeleteNoteResponse(const QJsonObject& obj)
{
}

void HttpManager::handleUpdateNoteResponse(const QJsonObject& obj)
{
}

void HttpManager::handleGetNoteByVersionResponse(const QJsonObject& obj)
{
}

void HttpManager::handleGetHistoryListResponset(const QJsonObject& obj)
{
}

