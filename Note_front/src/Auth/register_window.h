#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QWidget>

class HttpManager;
class QEvent;

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

private slots:
    void onRegisterClicked();
    void onRegisterResult(bool ok, const QString& message);
    void onNetworkError(const QString& error);

private:
    Ui::RegisterWindow* ui = nullptr;
    HttpManager* m_http = nullptr;
};

#endif // REGISTERWINDOW_H
