#include "mainwindow.h"

#include <QVBoxLayout>
#include <QWKWidgets/widgetwindowagent.h>

#include "login_window.h"
#include "register_window.h"
#include "editor_window.h"
#include "httpmanager.h"  
#include "title_bar.h"


MainWindow::MainWindow(HttpManager* http, QWidget* parent)
    : QMainWindow(parent),
      stacked_(new QStackedWidget(this))
{

    setWindowTitle("");
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

    stacked_ = new QStackedWidget(this);
    stacked_->addWidget(loginPage_);
    stacked_->addWidget(regPage_);
    stacked_->addWidget(editorPage_);
    stacked_->setCurrentWidget(loginPage_);

    // 新建一个 central 容器，把标题栏 + stacked_ 放进去
    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 标题栏，传入要控制的窗口 this
    auto* titleBar = new CustomTitleBar(this, central);
    // 想固定高度的话，可以：
    // titleBar->setFixedHeight(40);

    layout->addWidget(titleBar);
    layout->addWidget(stacked_);
    setCentralWidget(central);

    // ====== 接入 QWindowKit ======
    // 创建 agent，并尽量早调用 setup（官方推荐）
    auto* agent = new QWK::WidgetWindowAgent(this);
    agent->setup(this);

    // 告诉它哪个 widget 是标题栏
    agent->setTitleBar(titleBar);

    // 告诉它三个系统按钮（为了 Win11 Snap Layout 等系统行为）
    agent->setSystemButton(QWK::WindowAgentBase::Minimize, titleBar->minButton());
    agent->setSystemButton(QWK::WindowAgentBase::Maximize, titleBar->maxButton());
    agent->setSystemButton(QWK::WindowAgentBase::Close, titleBar->closeButton());

    // 如果以后在标题栏里加了 QMenuBar / 搜索框等，需要能正常点击的控件，
    // 就在这里标一下 hit-test 可见：
    // agent->setHitTestVisible(titleBar->menuBar(), true);

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
