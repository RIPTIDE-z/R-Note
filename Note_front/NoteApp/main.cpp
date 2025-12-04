#include <QApplication>
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

    QApplication a(argc, argv);

    
    HttpManager http;  
    //http.serBaseUrl("http://127.0.0.1:4523/m1/7498555-7234115-default");
    MainWindow w(&http);
    w.show();

    return a.exec();
}