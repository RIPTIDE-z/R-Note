#pragma once
#include <QMainWindow>
#include <QStackedWidget>

class NoteStructureManager;
class AppConfig;
class HttpManager;  
class LoginWindow;
class RegisterWindow;
class EditorWindow;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(AppConfig* config, HttpManager* http, NoteStructureManager* noteMgr, QWidget* parent = nullptr);

private:
    QStackedWidget* stacked_;
    LoginWindow* loginPage_;
    RegisterWindow* regPage_;
    EditorWindow* editorPage_;

    AppConfig* app_config_;
    HttpManager* http_manager_;
    NoteStructureManager* note_structure_manager_;

    static void resizeKeepCenter(QWidget* w, int newWidth, int newHeight);
};
