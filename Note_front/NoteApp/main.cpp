#include <QApplication>
#include <QGuiApplication>

#include "mainwindow.h"
#include "httpmanager.h"
#include "note_structure_manager.h"

int main(int argc, char* argv[]) {
    NoteStructureManager mgr;
    int id = 1;
    std::unique_ptr<NoteNode> node = mgr.buildFromDirectory(
        "D:/桌面/1/example",
        id,
        nullptr,
        QString()
    );

    mgr.saveToJsonFile("D:/桌面/1/test.json", node.get());

    QGuiApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    QApplication a(argc, argv);

    
    HttpManager http;  
    MainWindow w(&http);
    w.show();

    return a.exec();
}
