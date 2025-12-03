#include "editorwindow.h"
#include "httpmanager.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>

EditorWindow::EditorWindow(HttpManager* http, QWidget* parent)
    : QMainWindow(parent)
    , m_http(http)
{
    setWindowTitle("Note Editor");
    resize(800, 600);

    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // 中间一大片空白（将来放编辑器）
    auto* blank = new QLabel("Editor area (TODO)", this);
    blank->setAlignment(Qt::AlignCenter);
    blank->setStyleSheet("background-color: #202020; color: #aaaaaa;");
    mainLayout->addWidget(blank, 1);

    // 底部一行：左下角 Logout 按钮
    auto* bottomLayout = new QHBoxLayout();
    m_logoutButton = new QPushButton("Logout", this);
    bottomLayout->addWidget(m_logoutButton);
    bottomLayout->addStretch();

    mainLayout->addLayout(bottomLayout);

    connect(m_logoutButton, &QPushButton::clicked,
        this, &EditorWindow::onLogoutClicked);
    connect(m_http, &HttpManager::logoutResult,
        this, &EditorWindow::onLogoutResult);
    connect(m_http, &HttpManager::networkError,
        this, &EditorWindow::onNetworkError);
}

void EditorWindow::setToken(const QString& token)
{
    m_token = token;
}

void EditorWindow::onLogoutClicked()
{
    if (m_token.isEmpty()) {
        // 保险起见
        emit logoutSucceeded();
        return;
    }

    auto ret = QMessageBox::question(this,
        "Confirm logout",
        "确定要登出账号吗?");
    if (ret != QMessageBox::Yes) {
        return;
    }

    m_logoutButton->setEnabled(false);
    m_http->logout(m_token);
}

void EditorWindow::onLogoutResult(bool ok, const QString& message)
{
    m_logoutButton->setEnabled(true);

    if (!ok) {
        QMessageBox::warning(this, "Logout failed", message);
        return;
    }

    // 清掉 token，通知主程序切回登录界面
    m_token.clear();
    emit logoutSucceeded();
}

void EditorWindow::onNetworkError(const QString& error)
{
    m_logoutButton->setEnabled(true);
    QMessageBox::warning(this, "Network error", error);
}
