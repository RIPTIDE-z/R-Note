#include "register_window.h"
#include "ui_register_window.h"

#include "httpmanager.h"
#include "config_dialog.h"
#include "app_config.h"

#include <QEvent>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QWidget>

RegisterWindow::RegisterWindow(HttpManager* http,
    AppConfig* config,
    QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::RegisterWindow)
    , m_http(http)
    , m_config(config)
{
    ui->setupUi(this);

    // 提示文字初始为空
    ui->messageLabel->clear();

    // 手动设置动态属性 class="link"
    ui->signInLabel->setProperty("class", "link");
    ui->signInLabel->installEventFilter(this);

    // 信号槽
    connect(ui->registerButton, &QPushButton::clicked,
        this, &RegisterWindow::onRegisterClicked);
    connect(m_http, &HttpManager::registerResult,
        this, &RegisterWindow::onRegisterResult);
    connect(m_http, &HttpManager::networkError,
        this, &RegisterWindow::onNetworkError);
    connect(ui->configButton, &QPushButton::clicked,
        this, &RegisterWindow::onConfigButtonClicked);
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

    // 强制要求先配置好 Base URL 和 Project Root
    if (!m_config || m_config->projectRoot().isEmpty()) {
        ui->messageLabel->setText(
            QStringLiteral("请先点击“配置…”设置 Project Root"));
        return;
    }

    ui->messageLabel->clear();
    ui->registerButton->setEnabled(false);

    if (m_http) {
        m_http->registerUser(username, password);
    }
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

void RegisterWindow::onConfigButtonClicked()
{
    QWidget* win = window();   // 顶层窗口 MainWindow

    // 半透明遮罩
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
        this, &RegisterWindow::onConfigAccepted);

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

void RegisterWindow::onConfigAccepted(const QString& baseUrl,
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
