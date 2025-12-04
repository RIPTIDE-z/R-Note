#include "register_window.h"
#include "httpmanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QEvent> 

RegisterWindow::RegisterWindow(HttpManager* http, QWidget* parent)
    : QWidget(parent)
    , m_http(http)
{
    setWindowTitle("创建账号");
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

    auto* titleLabel = new QLabel("创建账号", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    auto* descLabel = new QLabel(
        "创建账号以同步你的笔记",
        this);
    descLabel->setWordWrap(true);
    descLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(descLabel);

    auto* nameLabel = new QLabel("用户名", this);
    m_usernameEdit = new QLineEdit(this);
    mainLayout->addWidget(nameLabel);
    mainLayout->addWidget(m_usernameEdit);

    auto* passLabel = new QLabel("密码", this);
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    mainLayout->addWidget(passLabel);
    mainLayout->addWidget(m_passwordEdit);

    m_messageLabel = new QLabel(this);
    mainLayout->addWidget(m_messageLabel);

    m_registerButton = new QPushButton("注册", this);
    mainLayout->addWidget(m_registerButton);

    mainLayout->addStretch();

    auto* bottomLayout = new QHBoxLayout();
    auto* tipLabel = new QLabel("已经有账号了？", this);
    auto* signInLabel = new QLabel("登录", this);
    signInLabel->setObjectName("signIn");
    signInLabel->setProperty("class", "link");
    signInLabel->setCursor(Qt::PointingHandCursor);
    signInLabel->installEventFilter(this);

    bottomLayout->addStretch();
    bottomLayout->addWidget(tipLabel);
    bottomLayout->addSpacing(4);
    bottomLayout->addWidget(signInLabel);
    bottomLayout->addStretch();

    mainLayout->addLayout(bottomLayout);

    connect(m_registerButton, &QPushButton::clicked,
        this, &RegisterWindow::onRegisterClicked);
    connect(m_http, &HttpManager::registerResult,
        this, &RegisterWindow::onRegisterResult);
    connect(m_http, &HttpManager::networkError,
        this, &RegisterWindow::onNetworkError);
}

bool RegisterWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (obj->objectName() == "signIn" &&
        event->type() == QEvent::MouseButtonRelease) {
        emit requestShowLogin();
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

void RegisterWindow::onRegisterClicked()
{
    const QString username = m_usernameEdit->text().trimmed();
    const QString password = m_passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        m_messageLabel->setText("用户名和密码不能为空");
        return;
    }

    m_messageLabel->clear();
    m_registerButton->setEnabled(false);
    m_http->registerUser(username, password);
}

void RegisterWindow::onRegisterResult(bool ok, const QString& message)
{
    m_registerButton->setEnabled(true);
    if (!ok) {
        m_messageLabel->setText(message);
        return;
    }
    m_messageLabel->setText("注册成功，你现在可以登录了");
}

void RegisterWindow::onNetworkError(const QString& error)
{
    m_registerButton->setEnabled(true);
    m_messageLabel->setText(error);
}
