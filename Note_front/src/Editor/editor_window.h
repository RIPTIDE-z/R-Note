#pragma once

#include <QMainWindow>
#include <memory>
#include <QJsonArray>
#include <QJsonObject>
#include <QPoint>

class AppConfig;
class HttpManager;
class QStandardItemModel;
class NoteStructureManager;
struct NoteNode;
class QModelIndex;
class QSplitter;
class MarkdownEditorWidget;
class MarkdownPreviewWidget;
class QListWidget;
class QListWidgetItem;
class QTextEdit;

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
    void onNoteUpdateClicked();

    // Treeview相关
    void onTreeItemDoubleClicked(const QModelIndex& index);
    // 右键菜单
    void onTreeViewContextMenuRequested(const QPoint& pos);
    void onHistoryContextMenuRequested(const QPoint& pos);
    
    // 服务端响应
    void onLogoutResult(bool ok, const QString& message);

    void onDeleteNoteResult(bool ok, const QString& msg);
    void onUpdateNoteResult(bool ok, const QString& msg, int remoteteId, const QString& localAbsPath);
    void onGetNoteByVersionResult(bool ok, const QString& msg, const QString& content);
    void onGetHistoryListResult(bool ok, const QString& msg, const QJsonArray& noteHistoryList);

    void onUpdateNoteStructureResult(bool ok, const QString& message);
    void onFetchNoteStructureResult(bool ok, const QString& message, const QJsonObject& noteStruct);
    void onNetworkError(const QString& error);

    // 历史预览：点击某一行后拉取指定版本内容 -> 显示到 previewHost
    void onHistoryItemClicked(QListWidgetItem* item);

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

    // 右侧三栏 splitter（editor | preview | history）
    QSplitter* m_rightSplitter = nullptr;

    // 主编辑器
    MarkdownEditorWidget* m_mainEditor = nullptr;

    // 历史内容预览（默认隐藏）
    MarkdownPreviewWidget* m_historyPreview = nullptr;

    // 历史列表 + 摘要
    QListWidget* m_historyList = nullptr;
    QTextEdit* m_summaryText = nullptr;

    // 当前打开笔记的 remoteId（用于按版本拉取）
    int m_currentRemoteNoteId = -1;
    // 当前笔记的绝对路径
    QString m_currentNoteAbsPath;

    // 大纲栏右键功能
    void createNoteUnderFolder(const QModelIndex& index);
    void createSubFolder(const QModelIndex& index);
    void deleteNote(const QModelIndex& index);
    void deleteFolder(const QModelIndex& index);

    // 控制 previewHost 显示/隐藏并调整 splitter 比例
    void setHistoryPreviewVisible(bool visible);

    // 弹窗输入 changeSummary（OK/Cancel）
    bool promptChangeSummary(const QString& title, const QString& hint, QString& outSummary);

    // 回滚流程的“挂起状态”（因为要先 getNoteByVersion，再 updateNote）
    bool m_pendingRollback = false;
    int m_pendingRollbackVersion = -1;
    QString m_pendingRollbackSummary;

    // ===== 远端删除的“挂起上下文” =====
    bool m_pendingDeleteRemote = false;
    int m_pendingDeleteNoteId = -1;          // 远端 noteId（remoteId）
    QString m_pendingDeleteAbsPath;          // 本地文件绝对路径
    QString m_pendingDeleteName;             // 笔记名（用于提示）
};
