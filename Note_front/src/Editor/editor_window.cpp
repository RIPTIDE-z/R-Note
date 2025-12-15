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
#include <QFileInfo>
#include <QSplitter>
#include <QList>
#include <QDir>
#include <QLineEdit>
#include <QMenu>
#include <QInputDialog>

#include "app_config.h"

EditorWindow::EditorWindow(HttpManager* http, AppConfig* config, QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::EditorWindow),
      m_http(http),
      m_config(config),
      m_structureMgr(new NoteStructureManager(this))
{
    ui->setupUi(this);

    // ================================= TreeView  =================================

    QFont f = ui->treeView->font();
    f.setPointSize(14); // 字号
    f.setFamily("JetBrains Mono"); // 字体名
    ui->treeView->setFont(f);

    // 双击展开
    ui->treeView->setExpandsOnDoubleClick(true);
    ui->treeView->setUniformRowHeights(true);
    ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 关闭水平滚动条，避免被压窄时横向滚动条闪一下
    ui->treeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, &QTreeView::customContextMenuRequested,
        this, &EditorWindow::onTreeViewContextMenuRequested);

    // =============================== 左侧 QSplitter ===============================

    // 使用 ui 中的 mainSplitter，不再手动移除 left/rightPanel
    m_splitter = ui->mainSplitter;
    if (m_splitter)
    {
        m_splitter->setHandleWidth(4);
        m_splitter->setStyleSheet(
            "QSplitter::handle:horizontal {"
            "    background: #444444;"
            "}"
            "QSplitter::handle:horizontal:hover {"
            "    background: #888888;"
            "}"
        );
        // 左右拉伸比例 1:3
        m_splitter->setStretchFactor(0, 1);
        m_splitter->setStretchFactor(1, 3);
    }

    // ======================== 在右侧加入 MarkdownEditorWidget ========================

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

    outerHLayout->addStretch(); // 左侧空白
    outerHLayout->addWidget(centerContainer);
    outerHLayout->addStretch(); // 右侧空白

    // 设置拉伸比例：1 : 8 : 1  => 中间 80%
    outerHLayout->setStretch(0, 1);
    outerHLayout->setStretch(1, 8);
    outerHLayout->setStretch(2, 1);

    ui->rightLayout->addLayout(outerHLayout);

    // ==================================== 信号连接 ====================================

    connect(ui->logoutButton, &QPushButton::clicked,
            this, &EditorWindow::onLogoutClicked);
    connect(ui->updateButton, &QPushButton::clicked,
            this, &EditorWindow::onUpdateClicked);
    connect(ui->syncButton, &QPushButton::clicked,
            this, &EditorWindow::onSyncClicked);

    connect(m_http, &HttpManager::logoutResult,
            this, &EditorWindow::onLogoutResult);

    connect(m_http, &HttpManager::deleteNoteResult,
        this, &EditorWindow::onDeleteNoteResult);
    connect(m_http, &HttpManager::updateNoteResult,
        this, &EditorWindow::onUpdateNoteResult);
    connect(m_http, &HttpManager::getNoteByVersioResult,
        this, &EditorWindow::onGetNoteByVersioResult);
    connect(m_http, &HttpManager::getHistoryListResult,
        this, &EditorWindow::onGetHistoryListResult);

    connect(m_http, &HttpManager::updateNoteStructureResult,
            this, &EditorWindow::onUpdateNoteStructureResult);
    connect(m_http, &HttpManager::fetchNoteStructureResult,
            this, &EditorWindow::onFetchNoteStructureResult);

    connect(m_http, &HttpManager::networkError,
            this, &EditorWindow::onNetworkError);

    //  Treeview双击逻辑
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

    // 每次更新前都重新扫描目录 + JSON，重建树并写入 json 文件
    initNoteTree(filePath, rootDir);

    // 直接把当前 m_rootNode 转为 JSON 上传
    const QJsonObject obj = m_structureMgr->toJson(m_rootNode.get());

    // 把最新结构上传给后端
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

void EditorWindow::onDeleteNoteResult(bool ok, const QString& msg)
{

}

void EditorWindow::onUpdateNoteResult(bool ok, const QString& msg, int remoteId, const QString& localAbsPath)
{
    if (!ok || remoteId <= 0 || localAbsPath.isEmpty())
        return;

    const QString rootDir = m_config->projectRoot();
    if (rootDir.isEmpty())
    {
        qDebug() << "projectRoot is empty in onFetchNoteStructureResult";
        return;
    }

    const QString filePath = rootDir + "/.Note/note_structure.json";

    const bool updated = m_structureMgr->setRemoteNoteIdByAbsolutePath(m_rootNode.get(), localAbsPath, remoteId);
    if (!updated) {
        // 找不到通常是因为树还没刷新/路径不一致
        return;
    }

    m_structureMgr->saveToJsonFile(filePath, m_rootNode.get());
}

void EditorWindow::onGetNoteByVersioResult(bool ok, const QString& msg, const QString& content)
{
}

void EditorWindow::onGetHistoryListResult(bool ok, const QString& msg, const QJsonArray& noteHistoryList)
{
}


void EditorWindow::onUpdateNoteStructureResult(bool ok, const QString& message)
{
    if (!ok)
    {
        QMessageBox::warning(this, "更新笔记结构失败", message);
        return;
    }
    QMessageBox::information(this, "更新笔记结构成功", message);
}

void EditorWindow::onFetchNoteStructureResult(bool ok, const QString& message, const QJsonObject& noteStruct)
{
    qDebug() << "fetch note structure result:" << ok << message;

    if (!ok)
    {
        // TODO: 弹对话框或状态栏提示
        // showMessage(message);
        return;
    }

    if (!m_config)
    {
        qDebug() << "Config is null in onFetchNoteStructureResult";
        return;
    }

    const QString rootDir = m_config->projectRoot();
    if (rootDir.isEmpty())
    {
        qDebug() << "projectRoot is empty in onFetchNoteStructureResult";
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

// 双击逻辑
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
    QString absolutePath = index.data(Qt::UserRole + 5).toString();

    qint64 remoteId = remoteIdVar.isValid() ? remoteIdVar.toLongLong() : -1;

    qDebug() << "Double clicked note:"
        << "id =" << id
        << ", remoteId =" << remoteId
        << ", fullPath =" << fullPath;

    // 切换编辑器显示
    m_mainEditor->setFilePath(absolutePath);
    m_mainEditor->loadFromFile();

    QMessageBox::information(
        this,
        "Open note",
        QString("id = %1\nremoteId = %2\nfullPath = %3\nabsolutePath = %4")
        .arg(id)
        .arg(remoteId)
        .arg(fullPath)
        .arg(absolutePath))
    ;
}

// 右键逻辑
void EditorWindow::onTreeViewContextMenuRequested(const QPoint& pos)
{
    if (!m_treeModel)
        return;

    QMenu menu(this);

    // 鼠标所在 index（可能是空白区域）
    QModelIndex index = ui->treeView->indexAt(pos);

    // ========= 空白区域：针对项目根目录 =========
    if (!index.isValid())
    {
        // 以根节点（第一行）作为目标文件夹
        if (m_treeModel->rowCount() <= 0)
            return;

        QModelIndex rootIndex = m_treeModel->index(0, 0);
        if (!rootIndex.isValid())
            return;

        QAction* actNewNote = menu.addAction(QStringLiteral("新建笔记"));
        QAction* actNewFolder = menu.addAction(QStringLiteral("新建文件夹"));

        connect(actNewNote, &QAction::triggered, this, [this, rootIndex]() {
            this->createNoteUnderFolder(rootIndex);
            });
        connect(actNewFolder, &QAction::triggered, this, [this, rootIndex]() {
            this->createSubFolder(rootIndex);
            });

        QPoint globalPos = ui->treeView->viewport()->mapToGlobal(pos);
        menu.exec(globalPos);
        return;
    }

    // ========= 点击在某个节点上 =========
    QString type = index.data(Qt::UserRole + 2).toString();

    if (type == "folder")
    {
        QAction* actNewNote = menu.addAction(QStringLiteral("新建笔记"));
        QAction* actNewSubFolder = menu.addAction(QStringLiteral("新建文件夹"));
        QAction* actDeleteFolder = menu.addAction(QStringLiteral("删除文件夹"));

        connect(actNewNote, &QAction::triggered, this, [this, index]() {
            this->createNoteUnderFolder(index);
            });
        connect(actNewSubFolder, &QAction::triggered, this, [this, index]() {
            this->createSubFolder(index);
            });
        connect(actDeleteFolder, &QAction::triggered, this, [this, index]() {
            this->deleteFolder(index);
            });
    }
    else if (type == "note")
    {
        QAction* actDelete = menu.addAction(QStringLiteral("删除笔记"));
        QAction* actUpdate = menu.addAction(QStringLiteral("更新窗口"));

        connect(actDelete, &QAction::triggered, this, [this, index]() {
            this->deleteNote(index);
            });
    }

    if (menu.actions().isEmpty())
        return;

    QPoint globalPos = ui->treeView->viewport()->mapToGlobal(pos);
    menu.exec(globalPos);
}

// 新建笔记
void EditorWindow::createNoteUnderFolder(const QModelIndex& index)
{
    QString folderPath = index.data(Qt::UserRole + 5).toString();
    if (folderPath.isEmpty())
    {
        qWarning() << "createNoteUnderFolder: folderPath is empty";
        return;
    }

    QInputDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("新建笔记"));
    dlg.setLabelText(QStringLiteral("笔记名称（不含扩展名）:"));
    dlg.setInputMode(QInputDialog::TextInput);
    dlg.setTextValue(QString());

    dlg.setStyleSheet(
        // 对话框整体
        "QInputDialog {"
        "    background-color: #201830;"
        "}"
        // label 区域
        "QLabel {"
        "    background-color: #201830;"
        "    color: #dddddd;"
        "}"
        // 输入框
        "QLineEdit {"
        "    background-color: #181020;"
        "    color: #dddddd;"
        "    border: 1px solid #444444;"
        "    border-radius: 4px;"
        "}"
    );

    // 关键：去掉系统边框
    dlg.setWindowFlags(dlg.windowFlags() | Qt::FramelessWindowHint);

    if (dlg.exec() != QDialog::Accepted)
        return;

    QString name = dlg.textValue().trimmed();
    if (name.isEmpty())
        return;

    QString fileName = name + ".md";
    QDir dir(folderPath);
    QString filePath = dir.filePath(fileName);

    if (QFileInfo::exists(filePath))
    {
        // 这里也可以改成无边框 QMessageBox，下一个步骤会教
        QMessageBox::warning(
            this,
            QStringLiteral("文件已存在"),
            QStringLiteral("同名笔记已存在，请换一个名字。")
        );
        return;
    }

    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::NewOnly | QIODevice::Text))
    {
        QMessageBox::warning(
            this,
            QStringLiteral("创建失败"),
            QStringLiteral("无法创建文件: %1\n错误: %2")
            .arg(filePath, f.errorString())
        );
        return;
    }

    QByteArray initial = QString("# %1\n").arg(name).toUtf8();

    // 将新笔记传送到服务器
    m_http->updateNote(
        m_token, 
        -1,
        2,
        -1,
        "Note init",
        initial,
        filePath
    );

    f.write(initial);
    f.close();

    if (!m_config)
    {
        qWarning() << "createNoteUnderFolder: m_config is null";
        return;
    }

    const QString rootDir = m_config->projectRoot();
    if (rootDir.isEmpty())
    {
        qWarning() << "createNoteUnderFolder: projectRoot is empty";
        return;
    }

    const QString jsonFile = rootDir + "/.Note/note_structure.json";
    initNoteTree(jsonFile, rootDir);
}

// 创建子文件夹
void EditorWindow::createSubFolder(const QModelIndex& index)
{
    QString parentPath = index.data(Qt::UserRole + 5).toString();
    if (parentPath.isEmpty())
    {
        qWarning() << "createSubFolder: parentPath is empty";
        return;
    }

    // 无边框输入框
    QInputDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("新建文件夹"));
    dlg.setLabelText(QStringLiteral("文件夹名称:"));
    dlg.setInputMode(QInputDialog::TextInput);
    dlg.setTextValue(QString());
    dlg.setWindowFlags(dlg.windowFlags() | Qt::FramelessWindowHint);

    dlg.setStyleSheet(
        // 对话框整体
        "QInputDialog {"
        "    background-color: #201830;"
        "}"
        // label 区域
        "QLabel {"
        "    background-color: #201830;"
        "    color: #dddddd;"
        "}"
        // 输入框
        "QLineEdit {"
        "    background-color: #181020;"
        "    color: #dddddd;"
        "    border: 1px solid #444444;"
        "    border-radius: 4px;"
        "}"
    );

    if (dlg.exec() != QDialog::Accepted)
        return;

    QString name = dlg.textValue().trimmed();
    if (name.isEmpty())
        return;

    QDir dir(parentPath);
    QString newFolderPath = dir.filePath(name);

    if (QFileInfo::exists(newFolderPath))
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setWindowTitle(QStringLiteral("已存在"));
        box.setText(QStringLiteral("同名文件夹已存在: %1").arg(name));
        box.setWindowFlags(box.windowFlags() | Qt::FramelessWindowHint);
        box.exec();
        return;
    }

    if (!dir.mkdir(name))
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setWindowTitle(QStringLiteral("创建失败"));
        box.setText(QStringLiteral("无法创建文件夹: %1").arg(newFolderPath));
        box.setWindowFlags(box.windowFlags() | Qt::FramelessWindowHint);
        box.exec();
        return;
    }

    // 创建成功后重新构建树 + 更新 json
    if (!m_config)
    {
        qWarning() << "createSubFolder: m_config is null";
        return;
    }

    const QString rootDir = m_config->projectRoot();
    if (rootDir.isEmpty())
    {
        qWarning() << "createSubFolder: projectRoot is empty";
        return;
    }

    const QString jsonFile = rootDir + "/.Note/note_structure.json";
    initNoteTree(jsonFile, rootDir);
}


// 删除笔记
void EditorWindow::deleteNote(const QModelIndex& index)
{
    QString absolutePath = index.data(Qt::UserRole + 5).toString();
    QString name = index.data(Qt::DisplayRole).toString();

    if (absolutePath.isEmpty())
    {
        qWarning() << "deleteNote: absolutePath is empty";
        return;
    }

    QMessageBox box(this);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle(QStringLiteral("删除笔记"));
    box.setText(QStringLiteral("确定要删除笔记 \"%1\" 吗？\n此操作会删除本地文件。").arg(name));
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);

    // 去掉系统边框
    box.setWindowFlags(box.windowFlags() | Qt::FramelessWindowHint);

    int ret = box.exec();
    if (ret != QMessageBox::Yes)
        return;

    QFile f(absolutePath);
    if (f.exists())
    {
        if (!f.remove())
        {
            QMessageBox errBox(this);
            errBox.setIcon(QMessageBox::Critical);
            errBox.setWindowTitle(QStringLiteral("删除失败"));
            errBox.setText(QStringLiteral("无法删除文件: %1\n错误: %2")
                .arg(absolutePath, f.errorString()));
            errBox.setWindowFlags(errBox.windowFlags() | Qt::FramelessWindowHint);
            errBox.exec();
            return;
        }
    }

    if (!m_config)
    {
        qWarning() << "deleteNote: m_config is null";
        return;
    }

    const QString rootDir = m_config->projectRoot();
    if (rootDir.isEmpty())
    {
        qWarning() << "deleteNote: projectRoot is empty";
        return;
    }

    const QString jsonFile = rootDir + "/.Note/note_structure.json";
    initNoteTree(jsonFile, rootDir);
}

void EditorWindow::deleteFolder(const QModelIndex& index)
{
    QString folderPath = index.data(Qt::UserRole + 5).toString();
    QString name = index.data(Qt::DisplayRole).toString();

    if (folderPath.isEmpty())
    {
        qWarning() << "deleteFolder: folderPath is empty";
        return;
    }

    QFileInfo info(folderPath);
    if (!info.isDir())
    {
        qWarning() << "deleteFolder: path is not a directory" << folderPath;
        return;
    }

    // 不允许删根目录（项目根）
    if (m_config && !m_config->projectRoot().isEmpty())
    {
        QString rootDir = QDir::cleanPath(m_config->projectRoot());
        QString target = QDir::cleanPath(folderPath);
        if (rootDir.compare(target, Qt::CaseInsensitive) == 0)
        {
            QMessageBox box(this);
            box.setIcon(QMessageBox::Warning);
            box.setWindowTitle(QStringLiteral("禁止操作"));
            box.setText(QStringLiteral("不能删除项目根目录。"));
            box.setWindowFlags(box.windowFlags() | Qt::FramelessWindowHint);
            box.exec();
            return;
        }
    }

    QMessageBox box(this);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle(QStringLiteral("删除文件夹"));
    box.setText(QStringLiteral("确定要删除文件夹 \"%1\" 及其内部所有笔记吗？\n此操作会递归删除本地文件。").arg(name));
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    box.setWindowFlags(box.windowFlags() | Qt::FramelessWindowHint);

    int ret = box.exec();
    if (ret != QMessageBox::Yes)
        return;

    QDir dir(folderPath);
    if (!dir.removeRecursively())
    {
        QMessageBox errBox(this);
        errBox.setIcon(QMessageBox::Critical);
        errBox.setWindowTitle(QStringLiteral("删除失败"));
        errBox.setText(QStringLiteral("无法递归删除文件夹: %1").arg(folderPath));
        errBox.setWindowFlags(errBox.windowFlags() | Qt::FramelessWindowHint);
        errBox.exec();
        return;
    }

    if (!m_config)
    {
        qWarning() << "deleteFolder: m_config is null";
        return;
    }

    const QString rootDir = m_config->projectRoot();
    if (rootDir.isEmpty())
    {
        qWarning() << "deleteFolder: projectRoot is empty";
        return;
    }

    const QString jsonFile = rootDir + "/.Note/note_structure.json";
    initNoteTree(jsonFile, rootDir);
}
