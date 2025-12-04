#include "editor_window.h"
#include "httpmanager.h"
#include "note_structure_manager.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>
#include <QTreeView>
#include <QHeaderView>
#include <QDebug>
#include <QStandardItemModel>

EditorWindow::EditorWindow(HttpManager* http, QWidget* parent)
    : QMainWindow(parent)
    , m_http(http)
    , m_structureMgr(new NoteStructureManager(this))
{
    setWindowTitle("Note Editor");
    resize(1000, 600);

    auto* central = new QWidget(this);
    setCentralWidget(central);

    // 顶层水平布局：左侧（TreeView + Logout），右侧（编辑区域）
    auto* mainHLayout = new QHBoxLayout(central);
    mainHLayout->setContentsMargins(16, 16, 16, 16);
    mainHLayout->setSpacing(12);

    // ========= 左侧：TreeView + Logout =========
    auto* leftLayout = new QVBoxLayout();
    leftLayout->setSpacing(8);

    m_treeView = new QTreeView(this);
    m_treeView->setHeaderHidden(false);
    m_treeView->setExpandsOnDoubleClick(true);
    m_treeView->setUniformRowHeights(true);
    m_treeView->setMinimumWidth(220);
    m_treeView->setAlternatingRowColors(true);
    m_treeView->setStyleSheet(
        "QTreeView {"
        "    background-color: #181818;"
        "    alternate-background-color: #202020;"
        "    color: #dddddd;"
        "    border: 1px solid #333333;"
        "}"
        "QTreeView::item:selected {"
        "    background-color: #3b2f5c;"
        "    color: #ffffff;"
        "}"
        "QTreeView::item:hover {"
        "    background-color: #252525;"
        "}"
        "QHeaderView::section {"
        "    background-color: #202020;"
        "    color: #cccccc;"
        "    border: none;"
        "    padding: 4px;"
        "}"
    );
    // 禁止编辑节点名
    m_treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    leftLayout->addWidget(m_treeView, 1);  // TreeView 占据左侧大部分高度

    // 左侧底部：登出按钮条（和 TreeView 同一列）
    auto* leftBottomBar = new QHBoxLayout();
    m_logoutButton = new QPushButton("登出", this);
    leftBottomBar->addWidget(m_logoutButton);
    leftBottomBar->addStretch();
    leftLayout->addLayout(leftBottomBar);

    mainHLayout->addLayout(leftLayout);

    // ========= 右侧：编辑区域 =========
    auto* rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(8);

    auto* blank = new QLabel("编辑区域", this);
    blank->setAlignment(Qt::AlignCenter);
    blank->setStyleSheet("background-color: #202020; color: #aaaaaa;");
    rightLayout->addWidget(blank, 1);

    mainHLayout->addLayout(rightLayout);

    // 设置左右宽度比例：左 1，右 3
    mainHLayout->setStretch(0, 1);  // 左：TreeView + Logout
    mainHLayout->setStretch(1, 3);  // 右：编辑区域

    // 连接信号
    connect(m_logoutButton, &QPushButton::clicked,
        this, &EditorWindow::onLogoutClicked);
    connect(m_http, &HttpManager::logoutResult,
        this, &EditorWindow::onLogoutResult);
    connect(m_http, &HttpManager::networkError,
        this, &EditorWindow::onNetworkError);

    connect(m_treeView, &QTreeView::doubleClicked,
        this, &EditorWindow::onTreeItemDoubleClicked);
}



void EditorWindow::setToken(const QString& token)
{
    m_token = token;
}

void EditorWindow::onLogoutClicked()
{
    if (m_token.isEmpty()) {
        // 保险起见
        emit logoutSucceeded();
        return;
    }

    auto ret = QMessageBox::question(this,
        "Confirm logout",
        "确定要登出账号吗?");
    if (ret != QMessageBox::Yes) {
        return;
    }

    m_logoutButton->setEnabled(false);
    m_http->logout(m_token);
}

void EditorWindow::onLogoutResult(bool ok, const QString& message)
{
    m_logoutButton->setEnabled(true);

    if (!ok) {
        QMessageBox::warning(this, "Logout failed", message);
        return;
    }

    // 清掉 token，通知主程序切回登录界面
    m_token.clear();
    emit logoutSucceeded();
}

void EditorWindow::onNetworkError(const QString& error)
{
    m_logoutButton->setEnabled(true);
    QMessageBox::warning(this, "Network error", error);
}

void EditorWindow::initNoteTree(const QString& jsonFilePath,
    const QString& rootDirPath)
{
    int nextId = 1;
    // 用 JSON + 目录生成最新的树结构
    m_rootNode = m_structureMgr->updateStructureFromDirAndJson(
        jsonFilePath,
        rootDirPath,
        nextId
    );

    if (!m_rootNode) {
        qDebug() << "initNoteTree: failed to build note structure tree.";
        // 可以考虑给 TreeView 设置空 model
        m_treeModel = new QStandardItemModel(m_treeView);
        m_treeModel->setHorizontalHeaderLabels({ "Name" });
        m_treeView->setModel(m_treeModel);
        return;
    }

    // 用 NoteStructureManager 内部的 createTreeModel 构建 model
    m_treeModel = m_structureMgr->createTreeModel(m_rootNode.get(), m_treeView);
    m_treeView->setModel(m_treeModel);

    // 可选：展开顶层
    m_treeView->expandToDepth(0);
    m_treeView->header()->setStretchLastSection(true);
}

void EditorWindow::onTreeItemDoubleClicked(const QModelIndex& index)
{
    if (!index.isValid() || !m_treeModel) return;

    // 取出 type
    QString type = index.data(Qt::UserRole + 2).toString();
    if (type != "note") {
        // 如果是 folder，双击就展开/收起即可，不做打开
        return;
    }

    QString id = index.data(Qt::UserRole + 1).toString();
    QVariant remoteIdVar = index.data(Qt::UserRole + 3);
    QString fullPath = index.data(Qt::UserRole + 4).toString();

    qint64 remoteId = remoteIdVar.isValid() ? remoteIdVar.toLongLong() : -1;

    // 这里暂时先给个提示，将来改成真正的“打开编辑器”
    qDebug() << "Double clicked note:"
        << "id =" << id
        << ", remoteId =" << remoteId
        << ", fullPath =" << fullPath;

    // 示例：可以先弹个对话框看结果
    
    QMessageBox::information(this,
                             "Open note",
                             QString("id = %1\nremoteId = %2\nfullPath = %3")
                                 .arg(id)
                                 .arg(remoteId)
                                 .arg(fullPath));
    

    // TODO: 在这里接你后面的编辑器逻辑：
    // 1. 从 fullPath 读取文件内容
    // 2. 或者根据 remoteId / id 去服务器请求内容
    // 3. 然后把内容显示到右边的编辑区域
}