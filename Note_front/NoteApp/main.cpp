#include <QApplication>
#include "mainwindow.h"
#include "httpmanager.h"

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    HttpManager http;  // 用你的 http 管理器
    //http.serBaseUrl("http://127.0.0.1:4523/m1/7498555-7234115-default");
    MainWindow w(&http);
    w.show();

    return a.exec();
}