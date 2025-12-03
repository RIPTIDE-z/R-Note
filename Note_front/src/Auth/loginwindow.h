#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>

class QLineEdit;
class QLabel;
class QPushButton;
class HttpManager;
class QJsonObject;

class LoginWindow : public QWidget
{
    Q_OBJECT
public:
    explicit LoginWindow(HttpManager* http, QWidget* parent = nullptr);

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
    HttpManager* m_http;
    QLineEdit* m_usernameEdit;
    QLineEdit* m_passwordEdit;
    QPushButton* m_loginButton;
    QLabel* m_messageLabel;
};

#endif // LOGINWINDOW_H
