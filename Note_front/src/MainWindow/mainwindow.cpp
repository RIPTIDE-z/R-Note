#include "mainwindow.h"
#include "login_window.h"
#include "register_window.h"
#include "editor_window.h"
#include "httpmanager.h"  


MainWindow::MainWindow(HttpManager* http, QWidget* parent)
    : QMainWindow(parent),
      stacked_(new QStackedWidget(this))
{

    setFixedSize(400, 400);


    setWindowTitle("R-Note");
    setFixedSize(500, 500);
    setStyleSheet(
        "QWidget { background-color: #111111; color: white; }"
        "QLineEdit { background-color: #1e1e1e; border: 1px solid #444444; "
        "border-radius: 4px; padding: 6px; }"
        "QPushButton { background-color: #7b3ff2; color: white; "
        "border-radius: 6px; padding: 8px; font-weight: 600; }"
        "QPushButton:disabled { background-color: #555555; }"
        "QLabel.link { color: #a26bff; }"
    );

    loginPage_ = new LoginWindow(http);
    regPage_ = new RegisterWindow(http);
    editorPage_ = new EditorWindow(http);

    // 添加到 stacked
    stacked_->addWidget(loginPage_);
    stacked_->addWidget(regPage_);
    stacked_->addWidget(editorPage_);

    setCentralWidget(stacked_);

    // 初始为登录页面
    stacked_->setCurrentWidget(loginPage_);

    // 信号连接：切换页面
    connect(loginPage_, &LoginWindow::requestShowRegister,
        this, [this]() { stacked_->setCurrentWidget(regPage_); });
    connect(regPage_, &RegisterWindow::requestShowLogin,
        this, [this]() { stacked_->setCurrentWidget(loginPage_); });
    connect(loginPage_, &LoginWindow::loginSucceeded,
        this, [&](const QString& token, const QJsonObject& noteStruct) {
            editorPage_->setToken(token);
            stacked_->setCurrentWidget(editorPage_);
        });
    connect(editorPage_, &EditorWindow::logoutSucceeded,
        this, [this]() {
            stacked_->setCurrentWidget(loginPage_);
        });

}
