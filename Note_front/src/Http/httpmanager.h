#pragma once

#include <QObject>
#include <QJsonObject>
#include <QNetworkAccessManager>

class QNetworkReply;

/**
 * 纯 HTTP 封装：
 * - 内部保存 m_baseUrl（通过 setBaseUrl 传入）
 * - 不保存 token / projectRoot，也不写任何文件
 * - 通过信号把 JSON 结果抛给上层
 */
class HttpManager : public QObject
{
    Q_OBJECT
public:
    explicit HttpManager(QObject* parent = nullptr);

    // 设置 baseUrl，例如 "http://127.0.0.1:8080/api"
    void setBaseUrl(const QString& url);

    // 用户注册登录相关
    // POST /auth/register
    void registerUser(const QString& username, const QString& password);

    // POST /auth/login
    void login(const QString& username, const QString& password);

    // POST /auth/logout
    void logout(const QString& token);

    // 笔记信息相关
    // DEL /notes/{noteId}
    void deleteNote(const QString& token, const int noteId);

    // DEL /notes/{noteId}
    void updateNote(const QString& token, const int noteId, const int code);

    // GET /notes/{noteId}/{version}
    void getNoteByVersion(const QString& token, const int noteId, const int version);

    // GET /notes/{noteId}/histories
    void getHistoryList(const QString& token, const int noteId);

    // 笔记结构相关
    // GET /note-structure
    void fetchNoteStructure(const QString& token);

    // PUT /note-structure
    void updateNoteStructure(const QString& token,
        const QJsonObject& noteStruct);       

signals:
    // 网络错误
    void networkError(const QString& msg);

    // 注册登录相关响应
    // 登录结果：ok, message, token, note_structure(可能为空对象)
    void loginResult(bool ok, const QString& msg,
        const QString& token,
        const QJsonObject& noteStruct);
    void registerResult(bool ok, const QString& msg);
    void logoutResult(bool ok, const QString& msg);

    // 笔记信息相关响应
    void deleteNoteResult(bool ok, const QString& msg);
    void updateNoteResult(bool ok, const QString& msg, int noteId);
    void getNoteByVersioResult(bool ok, const QString& msg, const QJsonObject& note);
    void getHistoryListResult(bool ok, const QString& msg, const QJsonObject& noteHistoryList);

    // 结构相关响应
    void fetchNoteStructureResult(
        bool ok,
        const QString& message,
        const QJsonObject& noteStructure
    );
    void updateNoteStructureResult(bool ok, const QString& message);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    // 响应体数据处理
    void handleLoginResponse(const QJsonObject& obj);
    void handleRegisterResponse(const QJsonObject& obj);
    void handleLogoutResponse(const QJsonObject& obj);
    void handleFetchNoteStructureResponse(const QJsonObject& obj);
    void handleUpdateNoteStructureResponse(const QJsonObject& obj);
    void handleDeleteNoteResponse(const QJsonObject& obj);
    void handleUpdateNoteResponse(const QJsonObject& obj);
    void handleGetNoteByVersionResponse(const QJsonObject& obj);
    void handleGetHistoryListResponset(const QJsonObject& obj);

private:
    QNetworkAccessManager m_manager;
    QString m_baseUrl;
};
