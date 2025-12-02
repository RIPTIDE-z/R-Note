#ifndef HTTPMANAGER_H
#define HTTPMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class HttpManager : public QObject
{
    Q_OBJECT

public:
    explicit HttpManager(QObject* parent = nullptr);
    ~HttpManager();

    // API 请求方法
    void getAllStudents();
    void getStudentById(int id);
    void addStudent(const QString& name, int age, const QString& className);
    void updateStudent(int id, const QString& name, int age, const QString& className);
    void deleteStudent(int id);

signals:
    void requestFinished(const QJsonObject& response);
    void requestError(const QString& error);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager* manager;
    QString baseUrl;

    void sendRequest(const QString& method, const QString& endpoint,
        const QJsonObject& data = QJsonObject());
};

#endif // HTTPMANAGER_H