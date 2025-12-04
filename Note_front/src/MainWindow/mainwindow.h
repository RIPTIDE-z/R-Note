#pragma once
#include <QMainWindow>
#include <QStackedWidget>

class HttpManager;  

class LoginWindow;
class RegisterWindow;
class EditorWindow;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(HttpManager* http, QWidget* parent = nullptr);

private:
    QStackedWidget* stacked_;
    LoginWindow* loginPage_;
    RegisterWindow* regPage_;
    EditorWindow* editorPage_;

    static void resizeKeepCenter(QWidget* w, int newWidth, int newHeight);
};
