#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include <QMainWindow>
#include <memory>

class HttpManager;
class QStandardItemModel;
class NoteStructureManager;
struct NoteNode;
class QModelIndex;
class QPropertyAnimation;    // 新增：前向声明动画类

namespace Ui {
    class EditorWindow;
}

class EditorWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit EditorWindow(HttpManager* http, QWidget* parent = nullptr);
    ~EditorWindow() override;

    void setToken(const QString& token);

    // 初始化左侧 TreeView 的结构：传入 json 文件和根目录
    void initNoteTree(const QString& jsonFilePath, const QString& rootDirPath);

signals:
    void logoutSucceeded();  // 通知主程序切回登录界面

private slots:
    void onLogoutClicked();
    void onLogoutResult(bool ok, const QString& message);
    void onNetworkError(const QString& error);
    void onTreeItemDoubleClicked(const QModelIndex& index);

    // 新增：点击“大纲”按钮时切换左侧大纲面板（展开/收起）
    void onOutlineButtonClicked();

private:
    Ui::EditorWindow* ui = nullptr;

    HttpManager* m_http = nullptr;
    QString m_token;

    QStandardItemModel* m_treeModel = nullptr;

    NoteStructureManager* m_structureMgr = nullptr;
    std::unique_ptr<NoteNode> m_rootNode;

    // ====== 新增：大纲面板动画相关成员 ======
    QPropertyAnimation* m_outlineAnim = nullptr;  // 控制 leftPanel 最大宽度的动画
    bool m_outlineVisible = true;                 // 当前大纲是否处于“展开显示”状态
    int  m_outlineTargetWidth = 260;             // 大纲展开后的目标宽度（可根据需要调整）
};

#endif // EDITORWINDOW_H
