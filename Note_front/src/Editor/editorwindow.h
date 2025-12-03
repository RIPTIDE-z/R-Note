#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include <QMainWindow>

class QPushButton;
class HttpManager;

class EditorWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit EditorWindow(HttpManager* http, QWidget* parent = nullptr);

    void setToken(const QString& token);

signals:
    void logoutSucceeded();  // 通知主程序切回登录界面

private slots:
    void onLogoutClicked();
    void onLogoutResult(bool ok, const QString& message);
    void onNetworkError(const QString& error);

private:
    HttpManager* m_http;
    QString m_token;
    QPushButton* m_logoutButton;
};

#endif // EDITORWINDOW_H
