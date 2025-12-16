#include "editor_window.h"
#include "ui_editor_window.h"

#include "httpmanager.h"
#include "note_structure_manager.h"
#include "markdown_editor_widget.h"
#include "markdown_preview_widget.h"
#include "app_config.h"

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
#include <QTimer>
#include <QSizePolicy>
#include <QIODevice>

#include <QListWidget>
#include <QLabel>
#include <QTextEdit>
#include <QFrame>
#include <QDateTime>
#include <algorithm>

// 把远端 fullPath（如 "/Root/a/b.md"）映射成本地绝对路径（rootDir + "/a/b.md"）
static QString remoteFullPathToLocalAbs(const QString& rootDir, const QString& fullPath)
{
    // fullPath: "/RemoteRoot/sub/a.md"
    QString p = fullPath;
    if (p.startsWith('/')) p.remove(0, 1);      // "RemoteRoot/sub/a.md"

    const int slash = p.indexOf('/');
    QString rel = (slash >= 0) ? p.mid(slash) : QString(); // "/sub/a.md" 或 ""

    return QDir::cleanPath(rootDir + rel);
}

// 将文本按照utf8编码写入文件，写入失败会把错误信息写到err
static bool writeUtf8TextFile(const QString& absPath, const QString& text, QString* err = nullptr)
{
    QFile f(absPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        if (err) *err = f.errorString();
        return false;
    }

    const QByteArray bytes = text.toUtf8();
    const qint64 n = f.write(bytes);
    f.close();

    if (n != bytes.size())
    {
        if (err) *err = QStringLiteral("write size mismatch");
        return false;
    }
    return true;
}

// 缩短时间显示
static QString formatCreatedTimeShort(const QString& iso)
{
    // 后端示例：2025-12-15T15:50:45.617364（可能是 6 位微秒）
    QDateTime dt = QDateTime::fromString(iso, Qt::ISODateWithMs);
    if (!dt.isValid())
        dt = QDateTime::fromString(iso, Qt::ISODate);

    if (dt.isValid())
        return dt.toLocalTime().toString("MM-dd HH:mm");

    QString s = iso;
    s.replace('T', ' ');
    if (s.size() >= 16) s = s.left(16);
    return s;
}

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

    // 右键单机设置
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, &QTreeView::customContextMenuRequested,
        this, &EditorWindow::onTreeViewContextMenuRequested);

    // =============================== 左侧 mainSplitter ===============================

    m_splitter = ui->mainSplitter;
    if (m_splitter)
    {
        m_splitter->setHandleWidth(4);
        m_splitter->setStyleSheet(
            "QSplitter::handle:horizontal { background: #444444;}"
            "QSplitter::handle:horizontal:hover { background: #888888;}"
        );
        // 左右拉伸比例 1:3
        m_splitter->setStretchFactor(0, 1);
        m_splitter->setStretchFactor(1, 3);
    }

    // =============================== 右侧 rightSplitter ===============================
    m_rightSplitter = ui->rightSplitter;
    if (m_rightSplitter)
    {
        m_rightSplitter->setHandleWidth(4);
        m_rightSplitter->setStyleSheet(
            "QSplitter::handle:horizontal { background: #444444; }"
            "QSplitter::handle:horizontal:hover { background: #888888; }"
        );

        // 允许 child 拖到 0
        m_rightSplitter->setChildrenCollapsible(true);
        m_rightSplitter->setCollapsible(1, true);   // preview 允许收缩到 0

        // 伸缩倾向（不等于最终宽度，最终由 setSizes 决定）
        m_rightSplitter->setStretchFactor(0, 6); // editor
        m_rightSplitter->setStretchFactor(1, 4); // preview
        m_rightSplitter->setStretchFactor(2, 3); // history
    }

    // =============================== editorHost：主编辑器 ===============================
    {
        auto* lay = ui->editorHostLayout;
        lay->setContentsMargins(0, 0, 0, 0);
        lay->setSpacing(0);

        m_mainEditor = new MarkdownEditorWidget(ui->editorHost);
        // editor 一般用 Expanding 即可
        m_mainEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        lay->addWidget(m_mainEditor);
    }

    // =============================== previewHost：历史版本内容预览（默认隐藏） ===============================
    {
        ui->previewHost->setObjectName("preview_widget");

        auto* lay = ui->previewHostLayout;
        lay->setContentsMargins(0, 0, 0, 0);
        lay->setSpacing(0);

        m_historyPreview = new MarkdownPreviewWidget(ui->previewHost);
        lay->addWidget(m_historyPreview);

        // 关键点1：不要 setVisible(false) 真隐藏
        // 否则用户没法通过 splitter 从 0 再把它拖出来（因为 widget 不参与布局了）
        ui->previewHost->setVisible(true);

        // 关键点2：解除“最小宽度/最小提示”对 splitter 的限制
        // QWidget/QLayout 通常不会把 widget 压到小于 minimumSizeHint，除非 minimumSize=0 或 sizePolicy=Ignore/ Ignored。:contentReference[oaicite:1]{index=1}
        ui->previewHost->setMinimumSize(0, 0);
        ui->previewHost->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);

        // preview 内部控件也做同样处理，避免它自身的 sizePolicy 限制拖到 0
        m_historyPreview->setMinimumSize(0, 0);
        m_historyPreview->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    }

    // =============================== historyHost：历史列表 + summary ===============================
    {
        ui->historyHost->setObjectName("history_widget");

        auto* hostLay = ui->historyHostLayout;
        hostLay->setContentsMargins(0, 0, 0, 0);
        hostLay->setSpacing(8);

        // 标题
        auto* title = new QLabel(QStringLiteral("History"), ui->historyHost);
        title->setStyleSheet("QLabel { color:#dddddd; font-weight:600; padding: 4px 2px; }");
        hostLay->addWidget(title);

        // 历史列表
        m_historyList = new QListWidget(ui->historyHost);
        m_historyList->setObjectName("history_list");
        m_historyList->setSelectionMode(QAbstractItemView::SingleSelection);
        m_historyList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        m_historyList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_historyList->setStyleSheet(
            "QListWidget {"
            "  background-color:#181818;"
            "  color:#dddddd;"
            "  border:1px solid #333333;"
            "}"
            "QListWidget::item {"
            "  padding: 8px 8px;"
            "  border-bottom: 1px solid #222222;"
            "}"
            "QListWidget::item:selected {"
            "  background-color:#3b2f5c;"
            "  color:#ffffff;"
            "}"
            "QListWidget::item:hover {"
            "  background-color:#252525;"
            "}"
        );

        QFont hf = m_historyList->font();
        hf.setFamily("JetBrains Mono");
        hf.setPointSize(12);
        m_historyList->setFont(hf);

        hostLay->addWidget(m_historyList, /*stretch*/ 1);

        // Summary 标题
        auto* sumTitle = new QLabel(QStringLiteral("Summary"), ui->historyHost);
        sumTitle->setStyleSheet("QLabel { color:#dddddd; font-weight:600; padding: 4px 2px; }");
        hostLay->addWidget(sumTitle);

        // Summary 文本
        m_summaryText = new QTextEdit(ui->historyHost);
        m_summaryText->setObjectName("summary_text");
        m_summaryText->setReadOnly(true);
        m_summaryText->setMinimumHeight(110);
        m_summaryText->setMaximumHeight(160);
        m_summaryText->setStyleSheet(
            "QTextEdit {"
            "  background-color:#181818;"
            "  color:#dddddd;"
            "  border:1px solid #333333;"
            "  padding: 6px;"
            "}"
        );
        m_summaryText->setPlaceholderText(QStringLiteral("双击左侧笔记以加载历史；点击历史行以预览版本内容。"));
        hostLay->addWidget(m_summaryText, /*stretch*/ 0);

        connect(m_historyList, &QListWidget::itemClicked,
            this, &EditorWindow::onHistoryItemClicked);

        // 开启右键菜单
        m_historyList->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_historyList, &QListWidget::customContextMenuRequested,
            this, &EditorWindow::onHistoryContextMenuRequested);
    }

    if (m_rightSplitter)
    {
        QTimer::singleShot(0, this, [this]() {
            // editor | preview(0) | history
            m_rightSplitter->setSizes({ 700, 0, 300 });
            });
    }

    // ==================================== 信号连接 ====================================

    connect(ui->logoutButton, &QPushButton::clicked,
            this, &EditorWindow::onLogoutClicked);
    connect(ui->updateButton, &QPushButton::clicked,
            this, &EditorWindow::onUpdateClicked);
    connect(ui->syncButton, &QPushButton::clicked,
            this, &EditorWindow::onSyncClicked);
    connect(ui->noteUpdateButton, &QPushButton::clicked,
        this, &EditorWindow::onNoteUpdateClicked);

    connect(m_http, &HttpManager::logoutResult,
            this, &EditorWindow::onLogoutResult);

    connect(m_http, &HttpManager::deleteNoteResult,
        this, &EditorWindow::onDeleteNoteResult);
    connect(m_http, &HttpManager::updateNoteResult,
        this, &EditorWindow::onUpdateNoteResult);
    connect(m_http, &HttpManager::getNoteByVersioResult,
        this, &EditorWindow::onGetNoteByVersionResult);
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

    // 初始：未选择笔记
    m_currentRemoteNoteId = -1;
}

EditorWindow::~EditorWindow()
{
    delete ui;
}

void EditorWindow::setToken(const QString& token)
{
    m_token = token;
}

void EditorWindow::setHistoryPreviewVisible(bool visible)
{
    if (!ui || !ui->previewHost || !m_rightSplitter)
        return;

    ui->previewHost->setVisible(visible);

    // 调整三栏比例
    if (!visible)
    {
        // editor + history 占满，preview 压到 0
        // 注意：sizes() 长度固定为 3
        QList<int> s = m_rightSplitter->sizes();
        if (s.size() == 3)
        {
            s[0] = s[0] + s[1]; // editor 吃掉 preview
            s[1] = 0;
            // s[2] 保持
            m_rightSplitter->setSizes(s);
        }
        else
        {
            m_rightSplitter->setSizes({ 800, 0, 300 });
        }
    }
    else
    {
        // 显示 preview：给一个合理初始比例
        m_rightSplitter->setSizes({ 600, 420, 320 });
    }
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

// 更新笔记版本
void EditorWindow::onNoteUpdateClicked()
{
    if (!m_http)
        return;

    if (m_token.isEmpty())
    {
        QMessageBox::warning(this, "Update note", "token为空，无法更新。");
        return;
    }

    if (m_currentRemoteNoteId <= 0 || m_currentNoteAbsPath.isEmpty())
    {
        QMessageBox::warning(this, "Update note", "请先在左侧双击打开一个笔记（需要 remoteId 和 absolutePath）。");
        return;
    }

    // ---------- 弹窗输入 changeSummary ----------
    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("更新笔记"));

    auto* layout = new QVBoxLayout(&dlg);

    auto* label = new QLabel(QStringLiteral("Change Summary（必填，简要描述这次更新）:"), &dlg);
    layout->addWidget(label);

    auto* edit = new QLineEdit(&dlg);
    edit->setPlaceholderText(QStringLiteral("例如：修正文档 / 添加xxx / 调整结构"));
    layout->addWidget(edit);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted)  // 用户点取消
        return;

    QString changeSummary = edit->text().trimmed();
    if (changeSummary.isEmpty())
    {
        QMessageBox::warning(this, "Update note", "changeSummary不能为空。");
        return;
    }

    // ---------- 获取 content：优先从编辑器内存拿（避免未保存改动丢失） ----------
    QString content;

    // 回退读文件（注意：若编辑器里有未保存改动，这里会读到旧内容）
    if (content.isEmpty())
    {
        const QString filePath = QDir::fromNativeSeparators(m_currentNoteAbsPath);
        QFile f(filePath);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QMessageBox::warning(this, "Update note",
                QStringLiteral("无法读取文件内容：%1\n%2").arg(filePath, f.errorString()));
            return;
        }
        content = QString::fromUtf8(f.readAll());
    }

    // ---------- 调后端更新 ----------
    const int code = 0;
    const int targetVersion = -1; 

    ui->noteUpdateButton->setEnabled(false);

    m_http->updateNote(
        m_token,
        m_currentRemoteNoteId,
        code,
        targetVersion,
        changeSummary,
        content,
        m_currentNoteAbsPath
    );
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
    if (ok) return;

    // 远端删失败
    QMessageBox box(this);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle(QStringLiteral("远端删除失败"));
    box.setText(msg);
    box.setWindowFlags(box.windowFlags() | Qt::FramelessWindowHint);
    box.exec();
}

void EditorWindow::onUpdateNoteResult(bool ok, const QString& msg, int remoteId, const QString& localAbsPath)
{
    if (!ok || remoteId <= 0 || localAbsPath.isEmpty())
    {
        ui->noteUpdateButton->setEnabled(true);
        return;
    }

    m_currentRemoteNoteId = remoteId;
    m_currentNoteAbsPath = localAbsPath;

    qDebug() << "接收到拉取结果";

    const QString rootDir = m_config->projectRoot();
    if (rootDir.isEmpty())
    {
        qDebug() << "projectRoot is empty in onFetchNoteStructureResult";
        return;
    }

    const QString filePath = rootDir + "/.Note/note_structure.json";

    const bool updated = m_structureMgr->setRemoteNoteIdByAbsolutePath(m_rootNode.get(), localAbsPath, remoteId);
    if (!updated) {
        return;
    }

    m_structureMgr->saveToJsonFile(filePath, m_rootNode.get());
    initNoteTree(filePath, rootDir);

    if (!m_token.isEmpty() && m_currentRemoteNoteId > 0)
    {
        qDebug() << "正在刷新历史列表";
        m_http->getHistoryList(m_token, m_currentRemoteNoteId);
    }

    ui->noteUpdateButton->setEnabled(true);
}

void EditorWindow::onHistoryItemClicked(QListWidgetItem* item)
{
    if (!item || !m_http)
        return;

    if (m_token.isEmpty() || m_currentRemoteNoteId <= 0)
    {
        m_summaryText->setPlainText(QStringLiteral("当前笔记没有 remoteId，或 token 为空，无法拉取历史版本。"));
        return;
    }

    const int version = item->data(Qt::UserRole).toInt();
    const QString summary = item->data(Qt::UserRole + 1).toString();

    m_summaryText->setPlainText(summary.isEmpty() ? QStringLiteral("(无 changeSummary)") : summary);

    setHistoryPreviewVisible(true);
    if (m_historyPreview)
        m_historyPreview->setMarkdownText(QStringLiteral("## Loading...\n正在拉取版本内容..."));

    // 拉取这个版本的内容
    m_http->getNoteByVersion(m_token, m_currentRemoteNoteId, version);
}

// 根据 m_isPullingMissingNotes m_pendingRollback 来判断是因为什么操作而
void EditorWindow::onGetNoteByVersionResult(bool ok, const QString& msg, const QString& content)
{
    // ============================= 拉取缺失笔记 =================================
    if (m_isPullingMissingNotes)
    {
        if (!ok)
        {
            qDebug() << "Pull missing note failed:" << msg
                << "remoteNoteId=" << m_currentPull.remoteNoteId
                << "absPath=" << m_currentPull.absPath;

            // 失败也继续下一个
            startNextMissingPull();
            return;
        }

        QString err;
        if (!writeUtf8TextFile(m_currentPull.absPath, content, &err))
        {
            qDebug() << "Write missing note failed:" << m_currentPull.absPath << err;
            // 写失败也继续下一个
        }

        startNextMissingPull();
        return;
    }

    // ================================ 回滚笔记 ====================================
    if (m_pendingRollback)
    {
        const int ver = m_pendingRollbackVersion;
        const QString summary = m_pendingRollbackSummary;

        // 清状态
        m_pendingRollback = false;
        m_pendingRollbackVersion = -1;
        m_pendingRollbackSummary.clear();

        if (!ok)
        {
            QMessageBox::warning(this, "Rollback", QString("拉取版本内容失败：%1").arg(msg));
            return;
        }

        if (m_token.isEmpty() || m_currentRemoteNoteId <= 0 || m_currentNoteAbsPath.isEmpty())
        {
            QMessageBox::warning(this, "Rollback", "token/noteId/path 无效，无法回滚。");
            return;
        }

        // 覆写本地文件
        QString err;
        if (!writeUtf8TextFile(m_currentNoteAbsPath, content, &err))
        {
            QMessageBox::warning(this, "Rollback", QString("覆写本地文件失败：%1").arg(err));
            return;
        }

        // 刷新主编辑器
        if (m_mainEditor)
        {
            m_mainEditor->setFilePath(m_currentNoteAbsPath);
            m_mainEditor->loadFromFile();
        }

        // 发起回滚
        m_http->updateNote(
            m_token,
            m_currentRemoteNoteId,
            1,
            ver,
            summary,
            content,
            m_currentNoteAbsPath
        );

        // 刷新历史列表
        m_http->getHistoryList(m_token, m_currentRemoteNoteId);

        return;
    }

    // ========================== 预览历史 ================================
    if (!m_historyPreview) return;

    if (!ok)
    {
        setHistoryPreviewVisible(true);
        m_historyPreview->setMarkdownText(QString("## 获取版本内容失败\n- %1").arg(msg));
        return;
    }

    setHistoryPreviewVisible(true);
    m_historyPreview->setMarkdownText(content);
}

void EditorWindow::onGetHistoryListResult(bool ok, const QString& msg, const QJsonArray& noteHistoryList)
{
    if (!m_historyList || !m_summaryText)
        return;

    m_historyList->clear();
    m_summaryText->clear();
    setHistoryPreviewVisible(false);

    if (!ok)
    {
        m_summaryText->setPlainText(QStringLiteral("获取历史列表失败：%1").arg(msg));
        return;
    }

    struct HItem { int version; QString createdTime; QString changeSummary; };
    std::vector<HItem> items;
    items.reserve(noteHistoryList.size());

    // 转为HItem结构
    for (const auto& v : noteHistoryList)
    {
        if (!v.isObject()) continue;
        const QJsonObject o = v.toObject();
        HItem it;
        it.version = o.value("version").toInt();
        it.createdTime = o.value("createdTime").toString();
        it.changeSummary = o.value("changeSummary").toString();
        items.push_back(std::move(it));
    }

    // 新版本在上（version 降序）
    std::sort(items.begin(), items.end(), [](const HItem& a, const HItem& b) {
        return a.version > b.version;
        });

    if (items.empty())
    {
        m_summaryText->setPlainText(QStringLiteral("该笔记暂无历史版本。"));
        return;
    }

    // 为每个HItem创建QListWidgetItem
    for (const auto& it : items)
    {
        const QString t = formatCreatedTimeShort(it.createdTime);
        const QString rowText = QString("v%1  %2").arg(it.version).arg(t);

        auto* item = new QListWidgetItem(rowText);
        item->setData(Qt::UserRole, it.version);
        item->setData(Qt::UserRole + 1, it.changeSummary);
        item->setToolTip(QString("version=%1\ncreated=%2\nsummary=%3")
            .arg(it.version)
            .arg(it.createdTime)
            .arg(it.changeSummary));

        m_historyList->addItem(item);
    }

    m_summaryText->setPlainText(QStringLiteral("历史列表已加载：点击某一行以预览该版本内容。"));
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
        return;

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

    if (m_token.isEmpty())
    {
        qDebug() << "Token is empty in onFetchNoteStructureResult";
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
            qDebug() << "Failed to create dir for note_structure.json:" << dir.absolutePath();
            return;
        }
    }

    // 把后端返回的结构写入本地 json 文件
    {
        QJsonDocument doc(noteStruct);
        QFile f(filePath);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            qDebug() << "Failed to open note_structure.json for write:" << filePath << f.errorString();
            return;
        }
        f.write(doc.toJson(QJsonDocument::Indented));
        f.close();
    }

    // 解析远端结构：用 fullPath 映射本地路径，找缺失并加入队
    int maxIdDummy = 0;
    std::unique_ptr<NoteNode> remoteRoot = m_structureMgr->fromJson(noteStruct, maxIdDummy, QString());
    if (!remoteRoot)
    {
        // 解析失败就退化为：至少重建一次树
        initNoteTree(filePath, rootDir);
        return;
    }

    m_missingPullQueue.clear();
    collectMissingFromRemoteTree(remoteRoot.get(), rootDir);

    // 没有缺失：直接重建树
    if (m_missingPullQueue.isEmpty())
    {
        initNoteTree(filePath, rootDir);
        return;
    }

    // 有缺失：开始串行拉取
    m_isPullingMissingNotes = true;
    m_missingPullRootDir = rootDir;
    m_missingPullJsonFile = filePath;

    startNextMissingPull();
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

    m_currentRemoteNoteId = static_cast<int>(remoteId);
    m_currentNoteAbsPath = absolutePath;

    // ====== 加载历史列表 ======
    if (m_historyList) m_historyList->clear();
    if (m_summaryText) m_summaryText->clear();
    setHistoryPreviewVisible(false);

    if (m_token.isEmpty() || m_currentRemoteNoteId <= 0)
    {
        if (m_summaryText)
            m_summaryText->setPlainText(QStringLiteral("该笔记没有 remoteId（或 token 为空），无法获取历史列表。"));
    }
    else
    {
        // 这里假设 HttpManager 已实现该方法，并发出 getHistoryListResult 信号
        m_http->getHistoryList(m_token, m_currentRemoteNoteId);
    }

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

    // 去掉系统边框
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

    m_mainEditor->setFilePath(filePath);
    m_mainEditor->loadFromFile();

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

    onUpdateClicked();
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
    
    onUpdateClicked();
}

// 删除笔记
// 远端本地删除为异步删除，不检验同步性
//TODO 同步删除
void EditorWindow::deleteNote(const QModelIndex& index)
{
    const QString demonstrateName = index.data(Qt::DisplayRole).toString();
    const QString absolutePath = index.data(Qt::UserRole + 5).toString();

    const QVariant remoteIdVar = index.data(Qt::UserRole + 3);
    const int remoteId = remoteIdVar.isValid() ? remoteIdVar.toInt() : -1;

    if (absolutePath.isEmpty())
    {
        qWarning() << "deleteNote: absolutePath is empty";
        return;
    }

    QMessageBox box(this);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle(QStringLiteral("删除笔记"));
    box.setText(QStringLiteral("确定要删除笔记 \"%1\" 吗？\n此操作会删除本地文件，并尝试删除远端笔记。").arg(demonstrateName));
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    box.setWindowFlags(box.windowFlags() | Qt::FramelessWindowHint);

    const int ret = box.exec();
    if (ret != QMessageBox::Yes)
        return;

    // 先发远端删除（异步，不等待结果）
    if (m_http && !m_token.isEmpty() && remoteId > 0)
    {
        m_http->deleteNote(m_token, remoteId); 
    }

    // 立刻删本地
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

    // 刷新树
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

bool EditorWindow::promptChangeSummary(const QString& title, const QString& hint, QString& outSummary)
{
    QDialog dlg(this);
    dlg.setWindowTitle(title);

    auto* layout = new QVBoxLayout(&dlg);

    auto* label = new QLabel(hint, &dlg);
    layout->addWidget(label);

    auto* edit = new QLineEdit(&dlg);
    edit->setPlaceholderText(QStringLiteral("例如：回滚到稳定版本 / 修复误操作"));
    layout->addWidget(edit);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted)
        return false;

    outSummary = edit->text().trimmed();
    return !outSummary.isEmpty();
}

void EditorWindow::onHistoryContextMenuRequested(const QPoint& pos)
{
    if (!m_historyList) return;

    // 只在点到某个 item 上才弹菜单
    QListWidgetItem* item = m_historyList->itemAt(pos);
    if (!item) return;

    if (m_token.isEmpty() || m_currentRemoteNoteId <= 0 || m_currentNoteAbsPath.isEmpty())
    {
        QMessageBox::warning(this, "Rollback", "当前笔记未绑定 remoteId / token 为空 / 路径为空，无法回滚。");
        return;
    }

    const int version = item->data(Qt::UserRole).toInt();

    QMenu menu(this);
    QAction* actRollback = menu.addAction(QStringLiteral("回滚到此版本"));

    QAction* chosen = menu.exec(m_historyList->viewport()->mapToGlobal(pos));
    if (chosen != actRollback)
        return;

    QString summary;
    if (!promptChangeSummary(QStringLiteral("回滚到版本 v%1").arg(version),
        QStringLiteral("请输入本次回滚的 changeSummary（必填）："),
        summary))
    {
        return;
    }

    // 挂起回滚状态：等 getNoteByVersion 回来后再 updateNote(code=1)
    m_pendingRollback = true;
    m_pendingRollbackVersion = version;
    m_pendingRollbackSummary = summary;

    // 先拉取目标版本内容，真正回滚逻辑在onGetNoteByVersionResult里
    m_http->getNoteByVersion(m_token, m_currentRemoteNoteId, version);
}

// 通过从远端json文件和项目目录解析出每个文件的绝对路径，根据绝对路径对比文件是否缺失
// 这种检验只检验了文件是否存在而无法检验是否进行了修改或者是否是原本的文件
//TODO 文件hash校验
void EditorWindow::collectMissingFromRemoteTree(const NoteNode* node, const QString& rootDir)
{
    if (!node) return;

    const QString abs = remoteFullPathToLocalAbs(rootDir, node->fullPath);

    if (node->type == NoteNodeType::Folder)
    {
        // 先确保远端结构中的目录在本地存在
        QDir d;
        d.mkpath(abs); // 递归创建目录

        for (const auto& child : node->children)
            collectMissingFromRemoteTree(child.get(), rootDir);

        return;
    }

    // 如果此文件存在则返回
    if (QFileInfo::exists(abs))
        return;

    // 没有 remoteNoteId 的 note 没法拉取内容
    if (!node->remoteNoteId.has_value())
    {
        qDebug() << "Missing local note but remoteNoteId is empty, skip:" << node->fullPath;
        return;
    }

    // 确保父目录存在
    QFileInfo fi(abs);
    QDir parentDir = fi.dir();
    if (!parentDir.exists())
        parentDir.mkpath(".");

    PendingPull p;
    p.remoteNoteId = static_cast<int>(*node->remoteNoteId);
    p.absPath = abs;
    // 将文件加入拉取队列
    m_missingPullQueue.enqueue(p);
}

void EditorWindow::startNextMissingPull()
{
    if (!m_isPullingMissingNotes)
        return;

    if (m_missingPullQueue.isEmpty())
    {
        // 拉取完毕：用“json + 本地目录扫描”重建树（让 absolutePath 正确）
        m_isPullingMissingNotes = false;
        initNoteTree(m_missingPullJsonFile, m_missingPullRootDir); 
        return;
    }

    // 拉取完成后拉取队列中下一个
    m_currentPull = m_missingPullQueue.dequeue();

    // version = -1 拉最新版本
    m_http->getNoteByVersion(m_token, m_currentPull.remoteNoteId, -1);
}
