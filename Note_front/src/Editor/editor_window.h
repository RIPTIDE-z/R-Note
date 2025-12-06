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

    // 初始化左侧 TreeView 的结构：传入 json 文件和根目录
    void updateNoteTree(const QString& jsonFilePath, const QString& rootDirPath);

signals:
    // 通知主程序切回登录界面
    void logoutSucceeded();

private slots:
    void onLogoutClicked();
    void onUpdateClicked();
    void onSyncClicked();
    void onLogoutResult(bool ok, const QString& message);
    void onNetworkError(const QString& error);
    void onTreeItemDoubleClicked(const QModelIndex& index);

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
