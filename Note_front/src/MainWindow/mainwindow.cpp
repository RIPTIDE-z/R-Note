#include "mainwindow.h"
#include "login_window.h"
#include "register_window.h"
#include "editor_window.h"
#include "httpmanager.h"  


MainWindow::MainWindow(HttpManager* http, QWidget* parent)
    : QMainWindow(parent),
      stacked_(new QStackedWidget(this))
{
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

    // ===== 硬编码测试用的 JSON 和根目录路径 =====
    const QString jsonPath = QStringLiteral("D:/桌面/1/test.json");
    const QString rootDir = QStringLiteral("D:/桌面/1/example");
    // 初始化左侧 TreeView（只为看渲染效果）
    editorPage_->initNoteTree(jsonPath, rootDir);

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
