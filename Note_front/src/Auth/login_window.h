#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>

class HttpManager;
class QJsonObject;
class QEvent;

namespace Ui {
    class LoginWindow;
}

class LoginWindow : public QWidget
{
    Q_OBJECT
public:
    explicit LoginWindow(HttpManager* http, QWidget* parent = nullptr);
    ~LoginWindow() override;

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

signals:
    void loginSucceeded(const QString& token, const QJsonObject& noteStructure);
    void requestShowRegister();

private slots:
    void onLoginClicked();
    void onLoginResult(bool ok,
        const QString& message,
        const QString& token,
        const QJsonObject& noteStructure);
    void onNetworkError(const QString& error);

private:
    Ui::LoginWindow* ui;
    HttpManager* m_http;
};

#endif // LOGINWINDOW_H
