#include "login_window.h"
#include "ui_login_window.h"

#include "httpmanager.h"

#include <QJsonObject>
#include <QEvent>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

LoginWindow::LoginWindow(HttpManager* http, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::LoginWindow)
    , m_http(http)
{
    ui->setupUi(this);

    // 初始提示信息置空
    ui->messageLabel->clear();

    // 手动设置动态属性 class="link"
    ui->createAccount->setProperty("class", "link");

    // “创建账户”标签安装事件过滤器，用于点击切换到注册窗口
    ui->createAccount->installEventFilter(this);

    // 信号槽
    connect(ui->loginButton, &QPushButton::clicked,
        this, &LoginWindow::onLoginClicked);

    connect(m_http, &HttpManager::loginResult,
        this, &LoginWindow::onLoginResult);
    connect(m_http, &HttpManager::networkError,
        this, &LoginWindow::onNetworkError);
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

bool LoginWindow::eventFilter(QObject* obj, QEvent* event)
{
    // 只处理 createAccount 标签的鼠标释放事件
    if (obj == ui->createAccount &&
        event->type() == QEvent::MouseButtonRelease) {
        emit requestShowRegister();
        return true;
    }

    return QWidget::eventFilter(obj, event);
}

void LoginWindow::onLoginClicked()
{
    const QString username = ui->usernameEdit->text().trimmed();
    const QString password = ui->passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        ui->messageLabel->setText(QStringLiteral("用户名和密码不能为空"));
        return;
    }

    ui->messageLabel->clear();
    ui->loginButton->setEnabled(false);
    m_http->login(username, password);
}

void LoginWindow::onLoginResult(bool ok,
    const QString& message,
    const QString& token,
    const QJsonObject& noteStructure)
{
    ui->loginButton->setEnabled(true);

    if (!ok) {
        ui->messageLabel->setText(message);
        return;
    }

    // 保持与你原来逻辑一致（原代码写的是“创建成功”，如需可改成“登录成功”）
    ui->messageLabel->setText(QStringLiteral("创建成功"));
    emit loginSucceeded(token, noteStructure);
}

void LoginWindow::onNetworkError(const QString& error)
{
    ui->loginButton->setEnabled(true);
    ui->messageLabel->setText(error);
}
