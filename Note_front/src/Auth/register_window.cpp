#include "register_window.h"
#include "ui_register_window.h"

#include "httpmanager.h"
#include "config_dialog.h"

#include <QEvent>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QWidget>

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
    connect(ui->configButton, &QPushButton::clicked,
        this, &RegisterWindow::onConfigButtonClicked);

    m_baseUrl.clear();
    m_projectRoot.clear();
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

    // 这里强制要求配置先完成
    if (m_baseUrl.isEmpty() || m_projectRoot.isEmpty()) {
        ui->messageLabel->setText(QStringLiteral("请先点击“配置…”设置 Base URL 和 Project Root"));
        return;
    }

    ui->messageLabel->clear();
    ui->registerButton->setEnabled(false);

    // 把 baseUrl 应用到 HttpManager
    if (m_http) {
        // 这里假定你有 HttpManager::setBaseUrl(const QString &)
        // 如果名字叫 setBaseUrl，自行改成 setBaseUrl
        m_http->setBaseUrl(m_baseUrl);
    }

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

// ===== 配置按钮逻辑 =====
void RegisterWindow::onConfigButtonClicked()
{
    QWidget* win = window();   // 顶层窗口 MainWindow

    // 半透明遮罩
    auto* overlay = new QWidget(win);
    overlay->setObjectName("ConfigOverlay");
    overlay->setStyleSheet("#ConfigOverlay { background-color: rgba(0, 0, 0, 160); }");
    overlay->setGeometry(win->rect());
    overlay->show();
    overlay->raise();

    ConfigDialog dlg(m_baseUrl, m_projectRoot, win);
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
    m_baseUrl = baseUrl;
    m_projectRoot = projectRoot;

    // 把 baseUrl 立刻应用到 HttpManager
    if (m_http) {
        m_http->setBaseUrl(baseUrl);
    }

    // 清掉之前的错误提示
    ui->messageLabel->clear();

    // 往上层（MainWindow）广播一份，如果你后面要在 EditorWindow 那边用 projectRoot
    emit configChanged(baseUrl, projectRoot);
}