#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include <QMainWindow>
#include <memory>

class QPushButton;
class HttpManager;
class QTreeView;
class QStandardItemModel;
class NoteStructureManager;
struct NoteNode;

class EditorWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit EditorWindow(HttpManager* http, QWidget* parent = nullptr);

    void setToken(const QString& token);

    // 初始化左侧 TreeView 的结构：传入 json 文件和根目录
    void initNoteTree(const QString& jsonFilePath,const QString& rootDirPath);

signals:
    void logoutSucceeded();  // 通知主程序切回登录界面

private slots:
    void onLogoutClicked();
    void onLogoutResult(bool ok, const QString& message);
    void onNetworkError(const QString& error);
    void onTreeItemDoubleClicked(const QModelIndex& index);

private:
    HttpManager* m_http;
    QString m_token;
    QPushButton* m_logoutButton;

    QTreeView* m_treeView = nullptr;
    QStandardItemModel* m_treeModel = nullptr;

    NoteStructureManager* m_structureMgr = nullptr; 
    std::unique_ptr<NoteNode> m_rootNode;
};

#endif // EDITORWINDOW_H
