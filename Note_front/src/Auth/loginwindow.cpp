#include "loginwindow.h"
#include "httpmanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QJsonObject>
#include <QEvent> 

LoginWindow::LoginWindow(HttpManager* http, QWidget* parent)
    : QWidget(parent)
    , m_http(http)
{
    setWindowTitle("登录");
    setMinimumSize(400, 360);
    setStyleSheet(
        "QWidget { background-color: #111111; color: white; }"
        "QLineEdit { background-color: #1e1e1e; border: 1px solid #444444; "
        "border-radius: 4px; padding: 6px; }"
        "QPushButton { background-color: #7b3ff2; color: white; "
        "border-radius: 6px; padding: 8px; font-weight: 600; }"
        "QPushButton:disabled { background-color: #555555; }"
        "QLabel.link { color: #a26bff; }"
    );

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(16);

    // 标题
    auto* titleLabel = new QLabel("登录你的账户", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    mainLayout->addSpacing(10);

    // Email
    auto* emailLabel = new QLabel("用户名", this);
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText("输入你的用户名");
    mainLayout->addWidget(emailLabel);
    mainLayout->addWidget(m_usernameEdit);

    // Password
    auto* passLabel = new QLabel("密码", this);
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("输入你的密码");

    mainLayout->addWidget(passLabel);
    mainLayout->addWidget(m_passwordEdit);

    // 提示信息
    m_messageLabel = new QLabel(this);
    mainLayout->addWidget(m_messageLabel);

    // Sign in 按钮
    m_loginButton = new QPushButton("登录", this);
    mainLayout->addWidget(m_loginButton);

    mainLayout->addStretch();

    // 底部 "Don't have an account? Create an account"
    auto* bottomLayout = new QHBoxLayout();
    auto* tipLabel = new QLabel("还没有账户？", this);
    auto* createLabel = new QLabel("创建账户", this);
    createLabel->setObjectName("createAccount");
    createLabel->setProperty("class", "link");
    createLabel->setCursor(Qt::PointingHandCursor);

    bottomLayout->addStretch();
    bottomLayout->addWidget(tipLabel);
    bottomLayout->addSpacing(4);
    bottomLayout->addWidget(createLabel);
    bottomLayout->addStretch();

    mainLayout->addLayout(bottomLayout);

    // 信号槽
    connect(m_loginButton, &QPushButton::clicked,
        this, &LoginWindow::onLoginClicked);
    connect(createLabel, &QLabel::linkActivated, this, [this]() {
        emit requestShowRegister();
        });
    // QLabel 没有 clicked，这里用 mousePressEvent 更简单：
    createLabel->installEventFilter(this);

    connect(m_http, &HttpManager::loginResult,
        this, &LoginWindow::onLoginResult);
    connect(m_http, &HttpManager::networkError,
        this, &LoginWindow::onNetworkError);
}

// 简单用 eventFilter 处理 "Create an account" 点击
bool LoginWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (obj->objectName() == "createAccount" &&
        event->type() == QEvent::MouseButtonRelease) {
        emit requestShowRegister();
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

void LoginWindow::onLoginClicked()
{
    const QString username = m_usernameEdit->text().trimmed();
    const QString password = m_passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        m_messageLabel->setText("用户名和密码不能为空");
        return;
    }

    m_messageLabel->clear();
    m_loginButton->setEnabled(false);
    m_http->login(username, password);
}

void LoginWindow::onLoginResult(bool ok,
    const QString& message,
    const QString& token,
    const QJsonObject& noteStructure)
{
    m_loginButton->setEnabled(true);

    if (!ok) {
        m_messageLabel->setText(message);
        return;
    }

    m_messageLabel->setText("创建成功");
    emit loginSucceeded(token, noteStructure);
}

void LoginWindow::onNetworkError(const QString& error)
{
    m_loginButton->setEnabled(true);
    m_messageLabel->setText(error);
}
