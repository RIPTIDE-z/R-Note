#include "editor_window.h"
#include "ui_editor_window.h"

#include "httpmanager.h"
#include "note_structure_manager.h"

#include <QPushButton>
#include <QMessageBox>
#include <QTreeView>
#include <QHeaderView>
#include <QDebug>
#include <QStandardItemModel>
#include <QAbstractItemView>
#include <QHBoxLayout>

EditorWindow::EditorWindow(HttpManager* http, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::EditorWindow)
    , m_http(http)
    , m_structureMgr(new NoteStructureManager(this))
{
    ui->setupUi(this);

    // 某些行为属性在代码里设置更直观
    ui->treeView->setExpandsOnDoubleClick(true);
    ui->treeView->setUniformRowHeights(true);
    ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 手动设置左右宽度比例：左 1，右 3
    // mainHLayout 是在 .ui 里命名的 QHBoxLayout
    ui->mainHLayout->setStretch(0, 1);  // 左列（TreeView + Logout）
    ui->mainHLayout->setStretch(1, 3);  // 右列（编辑区域）

    // 信号连接
    connect(ui->logoutButton, &QPushButton::clicked,
        this, &EditorWindow::onLogoutClicked);

    connect(m_http, &HttpManager::logoutResult,
        this, &EditorWindow::onLogoutResult);
    connect(m_http, &HttpManager::networkError,
        this, &EditorWindow::onNetworkError);

    connect(ui->treeView, &QTreeView::doubleClicked,
        this, &EditorWindow::onTreeItemDoubleClicked);
}

EditorWindow::~EditorWindow()
{
    delete ui;
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

    ui->logoutButton->setEnabled(false);
    m_http->logout(m_token);
}

void EditorWindow::onLogoutResult(bool ok, const QString& message)
{
    ui->logoutButton->setEnabled(true);

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
    ui->logoutButton->setEnabled(true);
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
        // 空 model
        if (m_treeModel) {
            delete m_treeModel;
            m_treeModel = nullptr;
        }
        m_treeModel = new QStandardItemModel(ui->treeView);
        m_treeModel->setHorizontalHeaderLabels({ "Name" });
        ui->treeView->setModel(m_treeModel);
        return;
    }

    // 用 NoteStructureManager 内部的 createTreeModel 构建 model
    if (m_treeModel) {
        delete m_treeModel;
        m_treeModel = nullptr;
    }
    m_treeModel = m_structureMgr->createTreeModel(m_rootNode.get(), ui->treeView);
    ui->treeView->setModel(m_treeModel);

    // 可选：展开顶层
    ui->treeView->expandToDepth(0);
    ui->treeView->header()->setStretchLastSection(true);
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

    qDebug() << "Double clicked note:"
        << "id =" << id
        << ", remoteId =" << remoteId
        << ", fullPath =" << fullPath;

    QMessageBox::information(this,
        "Open note",
        QString("id = %1\nremoteId = %2\nfullPath = %3")
        .arg(id)
        .arg(remoteId)
        .arg(fullPath));

    // TODO: 在这里接你后面的编辑器逻辑
}
