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

    // POST /auth/register
    void registerUser(const QString& username, const QString& password);

    // POST /auth/login
    void login(const QString& username, const QString& password);

    // POST /auth/logout
    void logout(const QString& token);
    // 笔记结构相关
    void fetchNoteStructure(const QString& token);                 
    void updateNoteStructure(const QString& token,
        const QJsonObject& noteStruct);       

signals:
    void networkError(const QString& msg);

    // 登录结果：ok, message, token, note_structure(可能为空对象)
    void loginResult(bool ok, const QString& msg,
        const QString& token,
        const QJsonObject& noteStruct);

    void registerResult(bool ok, const QString& msg);
    void logoutResult(bool ok, const QString& msg);

    // 笔记结构获取结果
    void noteStructureFetched(bool ok, const QString& msg,
        const QJsonObject& noteStruct);

    // 笔记结构更新结果
    void noteStructureUpdated(bool ok, const QString& msg);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    void handleLoginResponse(const QJsonObject& obj);
    void handleRegisterResponse(const QJsonObject& obj);
    void handleLogoutResponse(const QJsonObject& obj);
    void handleFetchNoteStructureResponse(const QJsonObject& obj);
    void handleUpdateNoteStructureResponse(const QJsonObject& obj);

private:
    QNetworkAccessManager m_manager;
    QString m_baseUrl;
};
