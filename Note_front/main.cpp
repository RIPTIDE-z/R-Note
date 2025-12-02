#include "mainwindow.h"
#include <QApplication>
#include <QFile>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    app.setStyle("fusion"); 

    QFile f(":/qss_qrc/qss/light_fluent.qss"); 
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app.setStyleSheet(QString::fromUtf8(f.readAll()));
    }

    app.setWindowIcon(QIcon(":/icon_qrc/icon/bf6.ico"));

    MainWindow window;
    window.setWindowTitle("BattleFiled6");
    window.show();

    return app.exec();
}