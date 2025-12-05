#include "register_window.h"
#include "ui_register_window.h"

#include "httpmanager.h"

#include <QEvent>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

RegisterWindow::RegisterWindow(HttpManager* http, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::RegisterWindow)
    , m_http(http)
{
    ui->setupUi(this);

    // 提示文字初始为空
    ui->messageLabel->clear();

    // 手动设置动态属性 class="link"
    ui->signInLabel->setProperty("class", "link");

    // “登录”标签安装事件过滤器
    ui->signInLabel->installEventFilter(this);

    // 信号槽
    connect(ui->registerButton, &QPushButton::clicked,
        this, &RegisterWindow::onRegisterClicked);
    connect(m_http, &HttpManager::registerResult,
        this, &RegisterWindow::onRegisterResult);
    connect(m_http, &HttpManager::networkError,
        this, &RegisterWindow::onNetworkError);
}

RegisterWindow::~RegisterWindow()
{
    delete ui;
}

bool RegisterWindow::eventFilter(QObject* obj, QEvent* event)
{
    // 只处理 signInLabel 的鼠标释放事件
    if (obj == ui->signInLabel &&
        event->type() == QEvent::MouseButtonRelease) {
        emit requestShowLogin();
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

void RegisterWindow::onRegisterClicked()
{
    const QString username = ui->usernameEdit->text().trimmed();
    const QString password = ui->passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        ui->messageLabel->setText(QStringLiteral("用户名和密码不能为空"));
        return;
    }

    ui->messageLabel->clear();
    ui->registerButton->setEnabled(false);
    m_http->registerUser(username, password);
}

void RegisterWindow::onRegisterResult(bool ok, const QString& message)
{
    ui->registerButton->setEnabled(true);
    if (!ok) {
        ui->messageLabel->setText(message);
        return;
    }
    ui->messageLabel->setText(QStringLiteral("注册成功，你现在可以登录了"));
}

void RegisterWindow::onNetworkError(const QString& error)
{
    ui->registerButton->setEnabled(true);
    ui->messageLabel->setText(error);
}
