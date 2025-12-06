#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QWidget>
#include <QString>

class HttpManager;
class QEvent;
class ConfigDialog;

namespace Ui {
    class RegisterWindow;
}

class RegisterWindow : public QWidget
{
    Q_OBJECT
public:
    explicit RegisterWindow(HttpManager* http, QWidget* parent = nullptr);
    ~RegisterWindow() override;

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

signals:
    void requestShowLogin();

    // 如果你以后想在 MainWindow 那边感知配置变化，可以用这个信号
    void configChanged(const QString& baseUrl,
        const QString& projectRoot);

private slots:
    void onRegisterClicked();
    void onRegisterResult(bool ok, const QString& message);
    void onNetworkError(const QString& error);

    // 新增：点击“配置…”按钮
    void onConfigButtonClicked();

    // 新增：ConfigDialog 返回成功时
    void onConfigAccepted(const QString& baseUrl,
        const QString& projectRoot);

private:
    Ui::RegisterWindow* ui = nullptr;
    HttpManager* m_http = nullptr;

    // 保存当前配置
    QString m_baseUrl;
    QString m_projectRoot;
};

#endif // REGISTERWINDOW_H
