#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include <QMainWindow>
#include <memory>

class AppConfig;
class HttpManager;
class QStandardItemModel;
class NoteStructureManager;
struct NoteNode;
class QModelIndex;
class QSplitter;

namespace Ui {
    class EditorWindow;
}

/**
 * 主编辑窗口：
 * - 左侧：大纲树（TreeView）
 * - 右侧：编辑区域（当前是一个占位 Label）
 * - 左右之间用 QSplitter 分隔，可用鼠标拖动改变比例
 * - 底部：通栏的“登出”按钮，一直可见
 */
class EditorWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit EditorWindow(HttpManager* http, AppConfig* config, QWidget* parent = nullptr);
    ~EditorWindow() override;

    void setToken(const QString& token);

    // 读取文件夹结构新建json文件并初始化Treeview
    void initNoteTree(const QString& jsonFilePath, const QString& rootDirPath);
    // 读取现有的json文件更新左侧 TreeView 
    void updateNoteTree(const QString& jsonFilePath, const QString& rootDirPath);

signals:
    // 通知主程序切回登录界面
    void logoutSucceeded();

private slots:
    // 按钮点击
    void onLogoutClicked();
    void onUpdateClicked();
    void onSyncClicked();
    void onTreeItemDoubleClicked(const QModelIndex& index);
    
    // 服务端响应
    void onLogoutResult(bool ok, const QString& message);
    void onUpdateResult(bool ok, const QString& message);
    void onFetchResult(bool ok, const QString& message, const QJsonObject& noteStruct);
    void onNetworkError(const QString& error);
    

private:
    Ui::EditorWindow* ui = nullptr;

    HttpManager* m_http = nullptr;
    QString m_token;
    AppConfig* m_config;

    QStandardItemModel* m_treeModel = nullptr;

    NoteStructureManager* m_structureMgr = nullptr;
    std::unique_ptr<NoteNode> m_rootNode;

    // 左右分栏的 splitter
    QSplitter* m_splitter = nullptr;
};

#endif // EDITORWINDOW_H
