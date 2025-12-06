#ifndef HTTPMANAGER_H
#define HTTPMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>

class HttpManager : public QObject
{
    Q_OBJECT
public:
    explicit HttpManager(QObject* parent = nullptr);

    // 用户注册/登录
    void registerUser(const QString& username, const QString& password);
    void login(const QString& username, const QString& password);
    void logout(const QString& token);
    // 笔记结构相关
    void fetchNoteStructure(const QString& token);                 
    void updateNoteStructure(const QString& token,
        const QJsonObject& noteStruct);       

    void setBaseUrl(const QString& url);

signals:
    // 注册结果
    void registerResult(bool ok, const QString& message);
    // 登录结果，ok 为 true 时带 token 和 note_structure
    void loginResult(bool ok,
        const QString& message,
        const QString& token,
        const QJsonObject& noteStructure);
    // 退出登录结果
    void logoutResult(bool ok, const QString& message);
    // 获取笔记结构
    void noteStructureFetched(bool ok, const QString& msg,
        const QJsonObject& noteStruct);
    // 笔记结构更新
    void noteStructureUpdated(bool ok, const QString& msg);
    // 通用网络错误
    void networkError(const QString& error);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager m_manager;
    QString m_baseUrl;

    void handleLoginResponse(const QJsonObject& obj);
    void handleRegisterResponse(const QJsonObject& obj);
    void handleLogoutResponse(const QJsonObject& obj);
    void handleFetchNoteStructureResponse(const QJsonObject& obj);
    void handleUpdateNoteStructureResponse(const QJsonObject& obj);

    void saveNoteStructureToFile(const QJsonObject& noteStruct);
};

#endif // HTTPMANAGER_H
