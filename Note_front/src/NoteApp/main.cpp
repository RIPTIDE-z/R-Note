#include <QApplication>
#include <QGuiApplication>
#include <QIcon>

#include "mainwindow.h"
#include "httpmanager.h"
#include "note_structure_manager.h"
#include "app_config.h"

int main(int argc, char* argv[])
{
    QGuiApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    QApplication a(argc, argv);

    // 设置程序Icon
    QIcon appIcon(":/icon/note_512.png");
    a.setWindowIcon(appIcon);

    // 1. 全局配置（运行时共享）
    AppConfig config;

    // 2. NoteStructure 管理器（只管 JSON <-> 树 <-> 文件）
    NoteStructureManager noteMgr;

    // 3. HTTP 管理器，依赖 AppConfig（拿 baseUrl / token）
    HttpManager http(&config);

    // 4. 主窗口，拿到这三者的指针
    MainWindow w(&config, &http, &noteMgr);
    w.setWindowIcon(appIcon);
    w.show();

    return a.exec();
}
