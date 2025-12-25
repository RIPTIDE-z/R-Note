#pragma once

#include <QObject>
#include <QString>

#include "notenode.h"

class HttpManager;
/**
 * 运行时配置类，统一保存：
 * - baseUrl：HTTP API 根地址，例如 http://127.0.0.1:8080/api
 * - projectRoot：本地笔记根目录
 * - token：当前已登录用户的 Auth-Token
 */
class AppConfig : public QObject {
    Q_OBJECT
   public:
    explicit AppConfig(QObject* parent = nullptr);

    QString baseUrl() const { return m_baseUrl; }
    QString projectRoot() const { return m_projectRoot; }
    QString token() const { return m_token; }

   public slots:
    void setBaseUrl(const QString& url);
    void setProjectRoot(const QString& root);
    void setToken(const QString& token);

   signals:
    void baseUrlChanged(const QString& url);
    void projectRootChanged(const QString& root);
    void tokenChanged(const QString& token);

   private:
    QString m_baseUrl;      // 服务端地址，例如 http://127.0.0.1:8080/api
    QString m_projectRoot;  // 当前的笔记项目根目录，例如 D:/Notes
    QString m_token;        // 登录成功后服务端返回的 Token
};
