#include <QApplication>
#include "httpmanager.h"
#include "loginwindow.h"
#include "registerwindow.h"
#include "editorwindow.h"
#include <QJsonObject>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icon_qrc/icon/bf6.ico"));

    HttpManager http;

    LoginWindow login(&http);
    RegisterWindow reg(&http);
    EditorWindow editor(&http);

    // 登录窗口 -> 显示注册窗口
    QObject::connect(&login, &LoginWindow::requestShowRegister,
        [&]() {
            reg.move(login.pos());
            reg.show();
            login.hide();
        });

    // 注册窗口 -> 显示登录窗口
    QObject::connect(&reg, &RegisterWindow::requestShowLogin,
        [&]() {
            login.move(reg.pos());
            login.show();
            reg.hide();
        });

    // 登录成功 -> 显示编辑界面，隐藏登录窗口
    QObject::connect(&login,
        &LoginWindow::loginSucceeded,
        [&](const QString& token, const QJsonObject&/*noteStruct*/) {
            editor.setToken(token);
            editor.show();
            login.hide();
        });

    // 编辑界面退出登录成功 -> 返回登录界面
    QObject::connect(&editor, &EditorWindow::logoutSucceeded,
        [&]() {
            editor.hide();
            login.show();
        });

    login.show();

    return app.exec();
}

