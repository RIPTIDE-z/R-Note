#pragma once

#include <QMainWindow>
#include <memory>

class AppConfig;
class HttpManager;
class QStandardItemModel;
class NoteStructureManager;
struct NoteNode;
class QModelIndex;
class QSplitter;
class MarkdownEditorWidget;

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

    // Treeview相关
    void onTreeItemDoubleClicked(const QModelIndex& index);
    // 右键菜单
    void onTreeViewContextMenuRequested(const QPoint& pos);
    
    // 服务端响应
    void onLogoutResult(bool ok, const QString& message);

    void onDeleteNoteResult(bool ok, const QString& msg);
    void onUpdateNoteResult(bool ok, const QString& msg, int remoteteId, const QString& localAbsPath);
    void onGetNoteByVersioResult(bool ok, const QString& msg, const QString& content);
    void onGetHistoryListResult(bool ok, const QString& msg, const QJsonArray& noteHistoryList);

    void onUpdateNoteStructureResult(bool ok, const QString& message);
    void onFetchNoteStructureResult(bool ok, const QString& message, const QJsonObject& noteStruct);
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

    // 新增：右侧主编辑器/预览控件
    MarkdownEditorWidget* m_mainEditor = nullptr;

    // 大纲栏右键功能
    void createNoteUnderFolder(const QModelIndex& index);
    void createSubFolder(const QModelIndex& index);
    void deleteNote(const QModelIndex& index);
    void deleteFolder(const QModelIndex& index);
};
