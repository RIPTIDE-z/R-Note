#include "app_config.h"

AppConfig::AppConfig(QObject* parent)
    : QObject(parent)
{
}

void AppConfig::setBaseUrl(const QString& url)
{
    if (m_baseUrl == url) {
        return;
    }
    m_baseUrl = url;
    emit baseUrlChanged(m_baseUrl);
}

void AppConfig::setProjectRoot(const QString& root)
{
    if (m_projectRoot == root) {
        return;
    }
    m_projectRoot = root;
    emit projectRootChanged(m_projectRoot);
}

void AppConfig::setToken(const QString& token)
{
    if (m_token == token) {
        return;
    }
    m_token = token;
    emit tokenChanged(m_token);
}
