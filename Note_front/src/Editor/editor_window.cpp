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
#include <QPropertyAnimation>
#include <QEasingCurve>

EditorWindow::EditorWindow(HttpManager* http, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::EditorWindow)
    , m_http(http)
    , m_structureMgr(new NoteStructureManager(this))
{
    ui->setupUi(this);

    QFont f = ui->treeView->font();
    f.setPointSize(14);                // 字号
    // 或者 f.setPixelSize(14);
    f.setFamily("JetBrain Mono");    // 字体家族，可选
    // f.setBold(true);               // 加粗，可选
    ui->treeView->setFont(f);


    // 某些行为属性在代码里设置更直观
    ui->treeView->setExpandsOnDoubleClick(true);
    ui->treeView->setUniformRowHeights(true);
    ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 关闭水平滚动条（例如你不想在动画时看到它闪出来）
    ui->treeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 手动设置左右宽度比例：左 1，右 3
    // mainHLayout 是在 .ui 里命名的 QHBoxLayout
    ui->mainHLayout->setStretch(0, 1);  // 左列（大纲面板）
    ui->mainHLayout->setStretch(1, 3);  // 右列（编辑区域）

    // ====== 新增：初始化大纲面板动画 ======
    // leftPanel 是 .ui 中包裹 TreeView + 登出按钮的 QWidget 容器
    // 通过动画它的 maximumWidth，在 0 和 m_outlineTargetWidth 之间插值 ->
    // 左侧大纲平滑伸缩，右侧编辑区域跟着自适应缩放
    ui->leftPanel->setMinimumWidth(0);                // 允许完全收起
    ui->leftPanel->setMaximumWidth(m_outlineTargetWidth);  // 默认展开状态的最大宽度
    m_outlineVisible = true;                          // 启动时默认显示大纲

    m_outlineAnim = new QPropertyAnimation(ui->leftPanel, "maximumWidth", this);
    m_outlineAnim->setDuration(220);                 // 动画时长（毫秒）
    m_outlineAnim->setEasingCurve(QEasingCurve::OutCubic); // 缓动曲线，使动画更自然

    // 信号连接
    connect(ui->logoutButton, &QPushButton::clicked,
        this, &EditorWindow::onLogoutClicked);

    connect(m_http, &HttpManager::logoutResult,
        this, &EditorWindow::onLogoutResult);
    connect(m_http, &HttpManager::networkError,
        this, &EditorWindow::onNetworkError);

    connect(ui->treeView, &QTreeView::doubleClicked,
        this, &EditorWindow::onTreeItemDoubleClicked);

    // ====== 新增：连接“大纲”按钮点击信号 ======
    connect(ui->outlineButton, &QPushButton::clicked,
        this, &EditorWindow::onOutlineButtonClicked);
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

// ====== 新增：大纲显示/隐藏的动画切换逻辑 ======
void EditorWindow::onOutlineButtonClicked()
{
    if (!m_outlineAnim)
        return;

    // 如果动画正在进行，先停掉，避免用户连点导致状态错乱
    if (m_outlineAnim->state() == QAbstractAnimation::Running) {
        m_outlineAnim->stop();
    }

    // 当前的最大宽度作为动画起点
    int startWidth = ui->leftPanel->maximumWidth();
    int endWidth = 0;

    if (!m_outlineVisible) {
        // 当前是隐藏状态 -> 目标是展开
        endWidth = m_outlineTargetWidth;
    }
    else {
        // 当前是展开状态 -> 目标是收起（宽度为 0）
        endWidth = 0;
    }

    m_outlineAnim->setStartValue(startWidth);
    m_outlineAnim->setEndValue(endWidth);
    m_outlineAnim->start();

    // 记录当前可见状态
    m_outlineVisible = !m_outlineVisible;
}
