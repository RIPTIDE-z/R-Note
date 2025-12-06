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
#include <QFile>
#include <QSplitter>
#include <QList>

#include "app_config.h"

EditorWindow::EditorWindow(HttpManager* http, AppConfig* config, QWidget* parent)
    : QMainWindow(parent),
    ui(new Ui::EditorWindow),
    m_http(http),
    m_config(config),
    m_structureMgr(new NoteStructureManager(this))
{
    ui->setupUi(this);

    // ========= 1. TreeView 外观与行为设置 =========
    {
        QFont f = ui->treeView->font();
        f.setPointSize(14);                    // 字号
        f.setFamily("JetBrains Mono");         // 字体名
        // f.setBold(true);                    // 如需加粗可打开
        ui->treeView->setFont(f);

        ui->treeView->setExpandsOnDoubleClick(true);
        ui->treeView->setUniformRowHeights(true);
        ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

        // 关闭水平滚动条，避免被压窄时横向滚动条闪一下
        ui->treeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    // ========= 2. 用 QSplitter 管左右两块 =========
    // .ui 中 centralLayout 下有：
    //  - mainHLayout（里面是 leftPanel / rightPanel）
    //  - bottomBarLayout（里面是 logoutButton）
    //
    // 这里把 leftPanel / rightPanel 从 mainHLayout 移出，
    // 放进一个水平 QSplitter，实现中间拖动分栏。

    QWidget* leftPanel = ui->leftPanel;
    QWidget* rightPanel = ui->rightPanel;

    // 从原来的 mainHLayout 中移除
    ui->mainHLayout->removeWidget(leftPanel);
    ui->mainHLayout->removeWidget(rightPanel);

    // 创建水平 splitter，作为 mainHLayout 唯一的子 widget
    m_splitter = new QSplitter(Qt::Horizontal, ui->centralwidget);
    m_splitter->setObjectName("editorSplitter");
    m_splitter->setHandleWidth(4);              // 分割条宽度

    // 分割条样式（可按需调整）
    m_splitter->setStyleSheet(
        "QSplitter::handle:horizontal {"
        "    background: #444444;"
        "}"
        "QSplitter::handle:horizontal:hover {"
        "    background: #888888;"
        "}"
    );

    // 把左右 panel 放进 splitter
    m_splitter->addWidget(leftPanel);
    m_splitter->addWidget(rightPanel);

    // 初始大小：近似 1:3
    {
        QList<int> sizes;
        sizes << 260 << 740;  // 左：大纲，右：编辑区
        m_splitter->setSizes(sizes);
    }

    // 清空 mainHLayout 里原有 item，只放一个 splitter
    while (ui->mainHLayout->count() > 0) {
        QLayoutItem* item = ui->mainHLayout->takeAt(0);
        if (!item->widget()) {
            delete item;
        }
    }
    ui->mainHLayout->addWidget(m_splitter);
    // 以后左右比例完全由 splitter 管理，不再使用 setStretch

    // ========= 3. 信号连接 =========
    connect(ui->logoutButton, &QPushButton::clicked,
        this, &EditorWindow::onLogoutClicked);
    connect(ui->updateButton, &QPushButton::clicked,
        this, &EditorWindow::onUpdateClicked);
    connect(ui->syncButton, &QPushButton::clicked,
        this, &EditorWindow::onSyncClicked);

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

// 登出逻辑
void EditorWindow::onLogoutClicked()
{
    if (m_token.isEmpty()) {
        // 保险起见，没 token 直接认为已登出
        emit logoutSucceeded();
        return;
    }

    auto ret = QMessageBox::question(
        this,
        "Confirm logout",
        "确定要登出账号吗?");
    if (ret != QMessageBox::Yes) {
        return;
    }

    ui->logoutButton->setEnabled(false);
    m_http->logout(m_token);
}

// 更新逻辑
void EditorWindow::onUpdateClicked()
{
    if (!m_config) {
        qDebug() << "Config is null, cannot update note structure";
        return;
    }

    const QString rootDir = m_config->projectRoot();
    if (rootDir.isEmpty()) {
        qDebug() << "projectRoot is empty, cannot update note structure";
        return;
    }

    const QString filePath = rootDir + "/.Note/note_structure.json";

    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) {
        qDebug() << "无法读取 json 文件，尝试新建：" << filePath << f.errorString();

        // 调用同一个类的创建逻辑，在这个路径下生成默认结构
        updateNoteTree(filePath, rootDir);

        // 新建之后再尝试打开
        if (!f.open(QIODevice::ReadOnly)) {
            qDebug() << "创建后仍无法打开 json 文件：" << filePath << f.errorString();
            return;
        }
    }

    QByteArray bytes = f.readAll();
    f.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(bytes, &err);
    if (err.error != QJsonParseError::NoError) {
        qDebug() << "JSON parse error:" << err.errorString();
        return;
    }
    if (!doc.isObject()) {
        qDebug() << "JSON root is not an object";
        return;
    }

    QJsonObject obj = doc.object();

    if (m_token.isEmpty()) {
        qDebug() << "Token is empty, cannot update";
        return;
    }

    // 这里直接复用 HttpManager 接口
    m_http->updateNoteStructure(m_token, obj);
}

// 拉取逻辑
void EditorWindow::onSyncClicked()
{
    
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

/**
 * @brief 根据Json文件和目录更新Treeview
 * @param jsonFilePath Json文件路径
 * @param rootDirPath 笔记根目录
 */
void EditorWindow::updateNoteTree(const QString& jsonFilePath,
                                  const QString& rootDirPath)
{
    int nextId = 1;

    // 用 JSON + 目录生成最新的树结构
    m_rootNode = m_structureMgr->updateStructureFromDirAndJson(
        jsonFilePath,
        rootDirPath,
        nextId
    );

    m_structureMgr->saveToJsonFile(jsonFilePath, m_rootNode.get());

    if (!m_rootNode) {
        qDebug() << "updateNoteTree: failed to build note structure tree.";
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

    // 展开顶层，最后一列自适应
    ui->treeView->expandToDepth(0);
    ui->treeView->header()->setStretchLastSection(true);
}

void EditorWindow::onTreeItemDoubleClicked(const QModelIndex& index)
{
    if (!index.isValid() || !m_treeModel)
        return;

    // 取出 type
    QString type = index.data(Qt::UserRole + 2).toString();
    if (type != "note") {
        // folder：双击只做展开/收起，不打开编辑
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

    QMessageBox::information(
        this,
        "Open note",
        QString("id = %1\nremoteId = %2\nfullPath = %3")
        .arg(id)
        .arg(remoteId)
        .arg(fullPath));

    // TODO: 在这里接你后面的编辑器逻辑
}
