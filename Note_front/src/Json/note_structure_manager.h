#pragma once
#include "notenode.h"

#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QStandardItemModel>
#include <QHash>

struct IdInfo {
    QString id;
    std::optional<qint64> remoteNoteId;
};

class NoteStructureManager : public QObject
{
    Q_OBJECT
public:
    explicit NoteStructureManager(QObject* parent = nullptr);

    // 从文件读取
    std::unique_ptr<NoteNode> loadFromJsonFile(const QString& filePath, int& maxIdOut);
    // 保存json到指定路径
    void saveToJsonFile(const QString& filePath, const NoteNode* root);

    std::unique_ptr<NoteNode> fromJson(const QJsonObject& obj, int& maxIdInOut,
        const QString& parentPath = QString());
    QJsonObject toJson(const NoteNode* node) const;

    // ========== 扫描目录，更新结构 ==========
    // existingIndex 可为空指针：表示全新生成
    std::unique_ptr<NoteNode> buildFromDirectory(
        const QString& rootDir,
        int& nextId,
        const QHash<QString, IdInfo>* existingIndex = nullptr,
        const QString& parentPath = QString()
    );

    // 从现有树构建 path -> {id, remoteNoteId} 的索引，用于更新时复用
    void buildPathIndex(const NoteNode* node,
        QHash<QString, IdInfo>& index,
        const QString& parentPath = QString()) const;

    // 高层封装：从现有 JSON + 本地目录生成“更新后”的树
    // 1. 解析 JSON 得到 oldRoot
    // 2. 用 oldRoot 构建 pathIndex
    // 3. 扫描目录，复用旧 id / remoteNoteId，生成 newRoot
    std::unique_ptr<NoteNode> updateStructureFromDirAndJson(
        const QString& jsonFile,
        const QString& rootDir,
        int& nextIdOut
    );

    // ========== 前端接口 ==========
// 生成 TreeView 用的数据模型
    QStandardItemModel* createTreeModel(const NoteNode* root, QObject* parent = nullptr) const;

private:
    QString generateId(int& nextId) const;
};
