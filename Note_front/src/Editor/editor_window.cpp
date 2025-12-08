#include "editor_window.h"
#include "ui_editor_window.h"

#include "httpmanager.h"
#include "note_structure_manager.h"
#include "markdown_editor_widget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QTreeView>
#include <QHeaderView>
#include <QDebug>
#include <QStandardItemModel>
#include <QAbstractItemView>
#include <QFile>
#include <qfileinfo.h>
#include <QSplitter>
#include <QList>
#include <QDir>

#include "app_config.h"

EditorWindow::EditorWindow(HttpManager* http, AppConfig* config, QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::EditorWindow),
      m_http(http),
      m_config(config),
      m_structureMgr(new NoteStructureManager(this))
{
    ui->setupUi(this);

    // ========= TreeView  =========
    {
        QFont f = ui->treeView->font();
        f.setPointSize(14); // 字号
        f.setFamily("JetBrains Mono"); // 字体名
        // f.setBold(true);                    // 如需加粗可打开
        ui->treeView->setFont(f);

        ui->treeView->setExpandsOnDoubleClick(true);
        ui->treeView->setUniformRowHeights(true);
        ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

        // 关闭水平滚动条，避免被压窄时横向滚动条闪一下
        ui->treeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    // ========= 用 QSplitter 管界面比例 =========
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
    m_splitter->setHandleWidth(4); // 分割条宽度

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
        sizes << 260 << 740; // 左：大纲，右：编辑区
        m_splitter->setSizes(sizes);
    }

    // 清空 mainHLayout 里原有 item，只放一个 splitter
    while (ui->mainHLayout->count() > 0)
    {
        QLayoutItem* item = ui->mainHLayout->takeAt(0);
        if (!item->widget())
        {
            delete item;
        }
    }
    ui->mainHLayout->addWidget(m_splitter);
    // 以后左右比例完全由 splitter 管理，不再使用 setStretch

    // ========= 在右侧加入 MarkdownEditorWidget =========

    // 中间的容器：以后你有多个 widget（编辑器 + 次级预览）也可以都放里面
    QWidget* centerContainer = new QWidget(ui->rightPanel);
    auto* centerVLayout = new QVBoxLayout(centerContainer);
    centerVLayout->setContentsMargins(0, 0, 0, 0);
    centerVLayout->setSpacing(0);

    // 真正的 markdown 编辑/预览控件
    m_mainEditor = new MarkdownEditorWidget(centerContainer);
    centerVLayout->addWidget(m_mainEditor);

    // 外层水平布局：左右留空，中间 80%
    auto* outerHLayout = new QHBoxLayout;
    outerHLayout->setContentsMargins(0, 0, 0, 0);
    outerHLayout->setSpacing(0);

    outerHLayout->addStretch();              // 左侧空白
    outerHLayout->addWidget(centerContainer);
    outerHLayout->addStretch();              // 右侧空白

    // 设置拉伸比例：1 : 8 : 1  => 中间 80%
    outerHLayout->setStretch(0, 1);
    outerHLayout->setStretch(1, 8);
    outerHLayout->setStretch(2, 1);

    // 把这个水平布局塞到 rightLayout 里
    ui->rightLayout->addLayout(outerHLayout);

    // 临时写死 md 文件路径
    const QString testPath = "D:/桌面/1/test.md";
    m_mainEditor->setFilePath(testPath);
    m_mainEditor->loadFromFile();


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
    connect(m_http, &HttpManager::updateNoteStructureResult,
            this, &EditorWindow::onUpdateResult);
    connect(m_http, &HttpManager::fetchNoteStructureResult,
            this, &EditorWindow::onFetchResult);

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
    if (m_token.isEmpty())
    {
        // 保险起见，没 token 直接认为已登出
        emit logoutSucceeded();
        return;
    }

    auto ret = QMessageBox::question(
        this,
        "Confirm logout",
        "确定要登出账号吗?");
    if (ret != QMessageBox::Yes)
    {
        return;
    }

    ui->logoutButton->setEnabled(false);
    m_http->logout(m_token);
}

// 更新逻辑：先重建本地结构，再上传
void EditorWindow::onUpdateClicked()
{
    // TODO:弹出确认窗口
    if (!m_config)
    {
        qDebug() << "Config is null, cannot update note structure";
        return;
    }

    const QString rootDir = m_config->projectRoot();
    if (rootDir.isEmpty())
    {
        qDebug() << "projectRoot is empty, cannot update note structure";
        return;
    }

    if (m_token.isEmpty())
    {
        qDebug() << "Token is empty, cannot update";
        return;
    }

    const QString filePath = rootDir + "/.Note/note_structure.json";

    // 确保目录存在
    QFileInfo info(filePath);
    QDir dir = info.dir();
    if (!dir.exists())
    {
        if (!dir.mkpath("."))
        {
            qDebug() << "Failed to create dir for note_structure.json:"
                << dir.absolutePath();
            return;
        }
    }

    // 1) 每次更新前都重新扫描目录 + JSON，重建树并写入 json 文件
    initNoteTree(filePath, rootDir);

    // 2) 读取刚刚生成的 json 文件
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly))
    {
        qDebug() << "创建/更新后仍无法读取 json 文件：" << filePath << f.errorString();
        return;
    }

    QByteArray bytes = f.readAll();
    f.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(bytes, &err);
    if (err.error != QJsonParseError::NoError)
    {
        qDebug() << "JSON parse error:" << err.errorString();
        return;
    }
    if (!doc.isObject())
    {
        qDebug() << "JSON root is not an object";
        return;
    }

    QJsonObject obj = doc.object();

    // 3) 把最新结构上传给后端
    m_http->updateNoteStructure(m_token, obj);
}


// 拉取逻辑
void EditorWindow::onSyncClicked()
{
    // TODO: 弹出确认窗口
    qDebug() << "尝试同步笔记";
    if (m_token.isEmpty())
    {
        qDebug() << "Token is empty, cannot fetch note structure";
        return;
    }
    if (!m_config)
    {
        qDebug() << "Config is null, cannot fetch note structure";
        return;
    }

    m_http->fetchNoteStructure(m_token);
    qDebug() << "fetch调用成功";
}

void EditorWindow::onLogoutResult(bool ok, const QString& message)
{
    ui->logoutButton->setEnabled(true);

    if (!ok)
    {
        QMessageBox::warning(this, "Logout failed", message);
        return;
    }

    // 清掉 token，通知主程序切回登录界面
    m_token.clear();
    emit logoutSucceeded();
}

void EditorWindow::onUpdateResult(bool ok, const QString& message)
{
    if (!ok)
    {
        QMessageBox::warning(this, "更新失败", message);
        return;
    }
    QMessageBox::information(this, "更新成功", message);
}

void EditorWindow::onFetchResult(bool ok, const QString& message, const QJsonObject& noteStruct)
{
    qDebug() << "fetch note structure result:" << ok << message;

    if (!ok)
    {
        // 这里可以弹个对话框或状态栏提示
        // showMessage(message);
        return;
    }

    if (!m_config)
    {
        qDebug() << "Config is null in onFetchResult";
        return;
    }

    const QString rootDir = m_config->projectRoot();
    if (rootDir.isEmpty())
    {
        qDebug() << "projectRoot is empty in onFetchResult";
        return;
    }

    const QString filePath = rootDir + "/.Note/note_structure.json";

    // 确保目录存在
    QFileInfo info(filePath);
    QDir dir = info.dir();
    if (!dir.exists())
    {
        if (!dir.mkpath("."))
        {
            qDebug() << "Failed to create dir for note_structure.json:"
                << dir.absolutePath();
            return;
        }
    }

    // 把后端返回的结构写入本地 json 文件
    QJsonDocument doc(noteStruct);
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qDebug() << "Failed to open note_structure.json for write:"
            << filePath << f.errorString();
        return;
    }
    f.write(doc.toJson(QJsonDocument::Indented));
    f.close();

    // 刷新左侧树 + 编辑区
    updateNoteTree(filePath, rootDir);
}

void EditorWindow::onNetworkError(const QString& error)
{
    ui->logoutButton->setEnabled(true);
    QMessageBox::warning(this, "Network error", error);
}

// 新建树
void EditorWindow::initNoteTree(const QString& jsonFilePath,
                                const QString& rootDirPath)
{
    int nextId = 1;

    // 用 JSON + 目录生成最新的树结构（会扫描文件夹）
    m_rootNode = m_structureMgr->updateStructureFromDirAndJson(
        jsonFilePath,
        rootDirPath,
        nextId
    );

    // 把合并后的树结构写回 jsonFilePath
    m_structureMgr->saveToJsonFile(jsonFilePath, m_rootNode.get());

    if (!m_rootNode)
    {
        qDebug() << "initNoteTree: failed to build note structure tree.";

        if (m_treeModel)
        {
            delete m_treeModel;
            m_treeModel = nullptr;
        }
        m_treeModel = new QStandardItemModel(ui->treeView);
        m_treeModel->setHorizontalHeaderLabels({"Name"});
        ui->treeView->setModel(m_treeModel);
        return;
    }

    // 重建 model
    if (m_treeModel)
    {
        delete m_treeModel;
        m_treeModel = nullptr;
    }
    m_treeModel = m_structureMgr->createTreeModel(m_rootNode.get(), ui->treeView);
    ui->treeView->setModel(m_treeModel);

    ui->treeView->expandToDepth(0);
    ui->treeView->header()->setStretchLastSection(true);
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

    // 只从 json 文件加载树结构，不扫描目录
    m_rootNode = m_structureMgr->loadFromJsonFile(jsonFilePath, nextId);
    if (!m_rootNode)
    {
        qDebug() << "updateNoteTree: failed to load note structure tree from json.";

        if (m_treeModel)
        {
            delete m_treeModel;
            m_treeModel = nullptr;
        }
        m_treeModel = new QStandardItemModel(ui->treeView);
        m_treeModel->setHorizontalHeaderLabels({"Name"});
        ui->treeView->setModel(m_treeModel);
        return;
    }

    if (m_treeModel)
    {
        delete m_treeModel;
        m_treeModel = nullptr;
    }
    m_treeModel = m_structureMgr->createTreeModel(m_rootNode.get(), ui->treeView);
    ui->treeView->setModel(m_treeModel);

    ui->treeView->expandToDepth(0);
    ui->treeView->header()->setStretchLastSection(true);
}

void EditorWindow::onTreeItemDoubleClicked(const QModelIndex& index)
{
    if (!index.isValid() || !m_treeModel)
        return;

    // 取出 type
    QString type = index.data(Qt::UserRole + 2).toString();
    if (type != "note")
    {
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
}
