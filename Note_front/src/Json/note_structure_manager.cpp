#include "note_structure_manager.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QDirIterator>
#include <QStandardItem>

NoteStructureManager::NoteStructureManager(QObject* parent)
    : QObject(parent)
{
}

/**
 * 生成自增ID
 * @param nextId 
 * @return 
 */
QString NoteStructureManager::generateId(int& nextId) const
{
    return QString::number(nextId++);
}

// ========== JSON解析 ==========
/**
 * @brief 将QJSON对象解析为NoteNode树结构
 * @param obj         要解析的QJson对象
 * @param maxIdInOut 
 * @param parentPath 
 * @return 
 */
std::unique_ptr<NoteNode> NoteStructureManager::fromJson(
    const QJsonObject& obj,
    int& maxIdInOut,
    const QString& parentPath
) {
    // 根节点
    auto node = std::make_unique<NoteNode>();

    // 写入根节点的数据
    node->id = obj.value("id").toString();
    node->name = obj.value("name").toString();
    QString typeStr = obj.value("type").toString();
    if (typeStr == "folder") {
        node->type = NoteNodeType::Folder;
    }
    else {
        node->type = NoteNodeType::Note;
    }

    // 更新 maxId，确保新的树节点的Id不会重复
    bool ok = false;
    int idVal = node->id.toInt(&ok);
    if (ok && idVal > maxIdInOut) {
        maxIdInOut = idVal;
    }

    // remoteNoteId (只有 note 才有)
    if (obj.contains("remoteNoteId")) {
        QJsonValue v = obj.value("remoteNoteId");
        if (v.isDouble()) {
            node->remoteNoteId = static_cast<qint64>(v.toDouble());
        }
    }

    // 更新当前路径
    QString currentPath = parentPath + "/" + node->name;
    node->fullPath = currentPath;

    if (node->type == NoteNodeType::Folder) {
        // 获取children数据，遍历处理
        const QJsonArray children = obj.value("children").toArray();
        // 预分配内存
        node->children.reserve(children.size());

        for (const QJsonValue& v : children) {
            if (!v.isObject()) continue;
            // 递归创建子节点
            auto child = fromJson(v.toObject(), maxIdInOut, currentPath);
            node->children.push_back(std::move(child));
        }
    }

    return node;
}

/**
 * @brief 解析出Json文件中的根节点
 * @param filePath 
 * @param maxIdOut 
 * @return 
 */
std::unique_ptr<NoteNode> NoteStructureManager::loadFromJsonFile(
    const QString& filePath, int& maxIdOut)
{
    maxIdOut = 0;
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) {
        return nullptr;
    }

    // 读取到原始字节格式
    const QByteArray data = f.readAll();
    f.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return nullptr;
    }

    QJsonObject rootObj = doc.object();
    // 返回从Json解析出的树顶层节点
    return fromJson(rootObj, maxIdOut, QString());
}

/**
 * 将NoteNode树结构解析为QJson对象
 * @param node 
 * @return 
 */
QJsonObject NoteStructureManager::toJson(const NoteNode* node) const
{
    QJsonObject obj;
    obj["id"] = node->id;
    obj["name"] = node->name;
    obj["type"] = (node->type == NoteNodeType::Folder) ? "folder" : "note";

    if (node->type == NoteNodeType::Note && node->remoteNoteId.has_value()) {
        obj["remoteNoteId"] = static_cast<qint64>(*node->remoteNoteId);
    }

    // 递归解析
    if (node->type == NoteNodeType::Folder) {
        QJsonArray arr;
        for (const auto& child : node->children) {
            arr.append(toJson(child.get()));
        }
        obj["children"] = arr;
    }

    return obj;
}

/**
 * 将NoteNode树结构存到Json文件
 * @param filePath 要保存到的文件路径
 * @param root     NoteNode根节点
 */
void NoteStructureManager::saveToJsonFile(const QString& filePath, const NoteNode* root)
{
    if (!root) {
        qDebug() << "saveToJsonFile: rootnode is null, skip.";
        return;
    }

    // 确保目录存在：若不存在则创建
    QFileInfo info(filePath);
    QDir dir = info.dir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qDebug() << "Failed to create dir for:" << filePath
                << ", error when making path.";
            return;
        }
    }

    // 先将树解析为QJSON对象
    QJsonObject rootObj = toJson(root);
    QJsonDocument doc(rootObj);

    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qDebug() << "Failed to open file for write:" << filePath << ", error:" << f.errorString();
        return;
    }
    f.write(doc.toJson(QJsonDocument::Indented));
    f.close();
    qDebug() << "Saved json to:" << filePath;
}


// ========== 用现有树构建 path -> IdInfo 索引 ==========
void NoteStructureManager::buildPathIndex(const NoteNode* node,
    QHash<QString, IdInfo>& index,
    const QString& parentPath) const
{
    if (!node) return;

    QString currentPath = parentPath + "/" + node->name;
    IdInfo info;
    info.id = node->id;
    info.remoteNoteId = node->remoteNoteId;

    index.insert(currentPath, info);

    if (node->type == NoteNodeType::Folder) {
        for (const auto& child : node->children) {
            buildPathIndex(child.get(), index, currentPath);
        }
    }
}

// ========== 扫描目录生成树，并尽量复用旧 ID ==========
std::unique_ptr<NoteNode> NoteStructureManager::buildFromDirectory(
    const QString& rootDir,
    int& nextId,
    const QHash<QString, IdInfo>* existingIndex,
    const QString& parentPath
) {
    QFileInfo rootInfo(rootDir);
    qDebug() << "rootDir =" << rootDir << "exists =" << rootInfo.exists();
    if (!rootInfo.exists()) {
        return nullptr;
    }

    auto node = std::make_unique<NoteNode>();
    node->name = rootInfo.fileName().isEmpty()
        ? rootInfo.absoluteFilePath()
        : rootInfo.fileName();
    node->type = rootInfo.isDir() ? NoteNodeType::Folder : NoteNodeType::Note;

    QString currentPath = parentPath + "/" + node->name;
    node->fullPath = currentPath;
    node->absolutePath = rootInfo.absoluteFilePath();

    // 复用旧 ID / remoteNoteId
    if (existingIndex && existingIndex->contains(currentPath)) {
        const IdInfo& info = existingIndex->value(currentPath);
        node->id = info.id;
        node->remoteNoteId = info.remoteNoteId;
    }
    else {
        node->id = generateId(nextId);
    }

    if (node->type == NoteNodeType::Folder) {
        QDir dir(rootDir);
        QFileInfoList entries = dir.entryInfoList(
            QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot,
            QDir::Name | QDir::IgnoreCase
        );

        for (const QFileInfo& fi : entries) {
            auto child = buildFromDirectory(fi.absoluteFilePath(), nextId, existingIndex, currentPath);
            if (child) {
                node->children.push_back(std::move(child));
            }
        }
    }

    // 如果是 note 且没有 remoteNoteId，就保持为空；后续由业务逻辑补充
    return node;
}

// ========== 高层封装：用 JSON + 目录生成最新结构 ==========
std::unique_ptr<NoteNode> NoteStructureManager::updateStructureFromDirAndJson(
    const QString& jsonFile,
    const QString& rootDir,
    int& nextIdOut
) {
    // 1. 读旧 JSON
    int maxId = 0;
    std::unique_ptr<NoteNode> oldRoot = loadFromJsonFile(jsonFile, maxId);
    nextIdOut = maxId + 1;

    // 2. 建索引
    QHash<QString, IdInfo> index;
    if (oldRoot) {
        buildPathIndex(oldRoot.get(), index, QString());
    }

    // 3. 扫描目录生成新树
    auto newRoot = buildFromDirectory(rootDir, nextIdOut, oldRoot ? &index : nullptr, QString());
    return newRoot;
}

// 往QStandardItem里填充数据
static void fillModelRecursive(QStandardItem* parentItem, const NoteNode* node)
{
    if (!node) return;

    // 第一列：名称
    auto* nameItem = new QStandardItem(node->name);

    // 附加数据
    nameItem->setData(node->id, Qt::UserRole + 1);
    nameItem->setData(
        node->type == NoteNodeType::Folder ? "folder" : "note",
        Qt::UserRole + 2
    );
    if (node->remoteNoteId.has_value()) {
        nameItem->setData(static_cast<qint64>(*node->remoteNoteId), Qt::UserRole + 3);
    }
    nameItem->setData(node->fullPath, Qt::UserRole + 4);
    nameItem->setData(node->absolutePath, Qt::UserRole + 5);

    parentItem->appendRow(nameItem);

    // 递归子节点
    if (node->type == NoteNodeType::Folder) {
        for (const auto& child : node->children) {
            fillModelRecursive(nameItem, child.get());
        }
    }
}

QStandardItemModel* NoteStructureManager::createTreeModel(
    const NoteNode* root,
    QObject* parent
) const
{
    auto* model = new QStandardItemModel(parent);
    model->setHorizontalHeaderLabels({ "Name" });

    if (!root) return model;

    // 根节点本身也作为一行
    auto* rootDummy = model->invisibleRootItem();
    fillModelRecursive(rootDummy, root);

    return model;
}

// 通过绝对路径在json中搜寻对应笔记并写入remoteId
bool NoteStructureManager::setRemoteNoteIdByAbsolutePath(NoteNode* node,
    const QString& absPath,
    const int remoteId)
{
    if (!node) return false;

    const QString a = QDir::cleanPath(node->absolutePath);
    const QString b = QDir::cleanPath(absPath);

    if (node->type == NoteNodeType::Note && a == b) {
        node->remoteNoteId = remoteId;
        return true;
    }

    if (node->type == NoteNodeType::Folder) {
        for (auto& child : node->children) {
            if (setRemoteNoteIdByAbsolutePath(child.get(), absPath, remoteId))
                return true;
        }
    }
    return false;
}
