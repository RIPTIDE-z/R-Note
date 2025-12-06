#include "mainwindow.h"

#include <QVBoxLayout>

#include "login_window.h"
#include "register_window.h"
#include "editor_window.h"
#include "httpmanager.h"  
#include "title_bar.h"


MainWindow::MainWindow(HttpManager* http, QWidget* parent)
    : QMainWindow(parent),
      stacked_(new QStackedWidget(this))
{
    // 1. 先去掉系统标题栏
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

    // 2. 原来的代码照写
    setWindowTitle("R-Note");
    resize(500, 500);
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

    const QString jsonPath = QStringLiteral("D:/桌面/1/test.json");
    const QString rootDir = QStringLiteral("D:/桌面/1/example");
    editorPage_->initNoteTree(jsonPath, rootDir);

    stacked_ = new QStackedWidget(this);
    stacked_->addWidget(loginPage_);
    stacked_->addWidget(regPage_);
    stacked_->addWidget(editorPage_);
    stacked_->setCurrentWidget(loginPage_);

    // 3. 新建一个 central 容器，把标题栏 + stacked_ 放进去
    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 标题栏，传入要控制的窗口 this
    auto* titleBar = new CustomTitleBar(this, central);
    // 想固定高度的话，可以自己设，比如：
    // titleBar->setFixedHeight(40);
    // 或用 qss: CustomTitleBar { min-height: 40px; }

    layout->addWidget(titleBar);
    layout->addWidget(stacked_);

    setCentralWidget(central);

    // 信号连接：切换页面
    connect(loginPage_, &LoginWindow::requestShowRegister,
        this, [this]() { stacked_->setCurrentWidget(regPage_); });

    connect(regPage_, &RegisterWindow::requestShowLogin,
        this, [this]() { stacked_->setCurrentWidget(loginPage_); });

    connect(loginPage_, &LoginWindow::loginSucceeded,
        this, [&](const QString& token, const QJsonObject& noteStruct) {
            editorPage_->setToken(token);
            // 进入编辑器时放大一点
            resizeKeepCenter(this, 1440, 900);
            stacked_->setCurrentWidget(editorPage_);
        });

    connect(editorPage_, &EditorWindow::logoutSucceeded,
        this, [this]() {
            // 回到登录界面时缩回去
            resizeKeepCenter(this, 500, 500);
            stacked_->setCurrentWidget(loginPage_);
        });

}

void MainWindow::resizeKeepCenter(QWidget* w, int newWidth, int newHeight)
{
    // 当前窗口的中心点（屏幕坐标）
    QPoint center = w->frameGeometry().center();

    // 改大小
    w->resize(newWidth, newHeight);

    // 用新的大小重新计算左上角，让中心点保持不变
    QRect newGeom = w->frameGeometry();
    newGeom.moveCenter(center);
    w->move(newGeom.topLeft());
}
