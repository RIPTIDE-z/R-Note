#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include <QMainWindow>
#include <memory>

class HttpManager;
class QStandardItemModel;
class NoteStructureManager;
struct NoteNode;
class QModelIndex;

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

private:
    Ui::EditorWindow* ui = nullptr;

    HttpManager* m_http = nullptr;
    QString m_token;

    QStandardItemModel* m_treeModel = nullptr;

    NoteStructureManager* m_structureMgr = nullptr;
    std::unique_ptr<NoteNode> m_rootNode;
};

#endif // EDITORWINDOW_H
