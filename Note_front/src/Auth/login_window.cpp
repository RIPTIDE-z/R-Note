#include "login_window.h"
#include "ui_login_window.h"

#include "httpmanager.h"
#include "config_dialog.h"
#include "app_config.h"

#include <QJsonObject>
#include <QEvent>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QWidget>

LoginWindow::LoginWindow(HttpManager* http,
    AppConfig* config,
    QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::LoginWindow)
    , m_http(http)
    , m_config(config)
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

    // “配置…”按钮 —— 与 RegisterWindow 相同逻辑
    connect(ui->configButton, &QPushButton::clicked,
        this, &LoginWindow::onConfigButtonClicked);
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

    // 强制要求先配置好 Base URL 和 Project Root（和注册窗口保持一致）
    if (!m_config ||
        m_config->baseUrl().isEmpty() ||
        m_config->projectRoot().isEmpty()) {
        ui->messageLabel->setText(
            QStringLiteral("请先点击“配置…”设置 Base URL 和 Project Root"));
        return;
    }

    ui->messageLabel->clear();
    ui->loginButton->setEnabled(false);

    if (m_http) {
        m_http->login(username, password);
    }
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

    // 如有需要可改为“登录成功”
    ui->messageLabel->setText(QStringLiteral("创建成功"));
    emit loginSucceeded(token, noteStructure);
}

void LoginWindow::onNetworkError(const QString& error)
{
    ui->loginButton->setEnabled(true);
    ui->messageLabel->setText(error);
}

void LoginWindow::onConfigButtonClicked()
{
    QWidget* win = window();   // 顶层窗口 MainWindow

    // 半透明遮罩（和 RegisterWindow 完全一样）
    auto* overlay = new QWidget(win);
    overlay->setObjectName("ConfigOverlay");
    overlay->setStyleSheet(
        "#ConfigOverlay { background-color: rgba(0, 0, 0, 160); }");
    overlay->setGeometry(win->rect());
    overlay->show();
    overlay->raise();

    const QString baseUrl =
        m_config ? m_config->baseUrl() : QString();
    const QString projectRoot =
        m_config ? m_config->projectRoot() : QString();

    ConfigDialog dlg(baseUrl, projectRoot, win);
    connect(&dlg, &ConfigDialog::configAccepted,
        this, &LoginWindow::onConfigAccepted);

    // 居中
    dlg.adjustSize();
    const QRect frameGeom = win->frameGeometry();
    const QSize dlgSize = dlg.size();
    const QPoint center = frameGeom.center()
        - QPoint(dlgSize.width() / 2, dlgSize.height() / 2);
    dlg.move(center);

    dlg.exec();
    overlay->deleteLater();
}

void LoginWindow::onConfigAccepted(const QString& baseUrl,
    const QString& projectRoot)
{
    if (m_config) {
        m_config->setBaseUrl(baseUrl);
        m_config->setProjectRoot(projectRoot);
    }

    // 清掉之前的错误提示
    ui->messageLabel->clear();

    emit configChanged(baseUrl, projectRoot);
}
