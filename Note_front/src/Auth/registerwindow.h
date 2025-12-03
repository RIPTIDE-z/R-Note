#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QWidget>

class QLineEdit;
class QLabel;
class QPushButton;
class HttpManager;

class RegisterWindow : public QWidget
{
    Q_OBJECT
public:
    explicit RegisterWindow(HttpManager* http, QWidget* parent = nullptr);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

signals:
    void requestShowLogin();

private slots:
    void onRegisterClicked();
    void onRegisterResult(bool ok, const QString& message);
    void onNetworkError(const QString& error);

private:
    HttpManager* m_http;
    QLineEdit* m_usernameEdit;
    QLineEdit* m_passwordEdit;
    QPushButton* m_registerButton;
    QLabel* m_messageLabel;
};

#endif // REGISTERWINDOW_H
