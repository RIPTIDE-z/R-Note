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

    // 用户注册
    void registerUser(const QString& username, const QString& password);
    // 用户登录
    void login(const QString& username, const QString& password);
    // 用户退出登录
    void logout(const QString& token);

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

    void saveNoteStructureToFile(const QJsonObject& noteStruct);
};

#endif // HTTPMANAGER_H
