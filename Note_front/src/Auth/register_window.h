#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QWidget>
#include <QString>

class HttpManager;
class QEvent;
class ConfigDialog;
class AppConfig;

namespace Ui {
    class RegisterWindow;
}

class RegisterWindow : public QWidget
{
    Q_OBJECT
public:
    explicit RegisterWindow(HttpManager* http,
        AppConfig* config,
        QWidget* parent = nullptr);
    ~RegisterWindow() override;

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

signals:
    void requestShowLogin();

private slots:
    void onRegisterClicked();
    void onRegisterResult(bool ok, const QString& message);
    void onNetworkError(const QString& error);

    // “配置…”按钮
    void onConfigButtonClicked();

    // ConfigDialog 返回成功时
    void onConfigAccepted(const QString& baseUrl,
        const QString& projectRoot);

private:
    Ui::RegisterWindow* ui = nullptr;
    HttpManager* m_http = nullptr;
    AppConfig* m_config = nullptr;
};

#endif // REGISTERWINDOW_H
