#pragma once

#include "notenode.h"

#include <QObject>
#include <QString>
#include <QHash>

#include <memory>
#include <vector>
#include <optional>

class QJsonObject;
class QStandardItemModel;

struct IdInfo
{
    QString id;
    std::optional<qint64> remoteNoteId;
};

/**
 * 只负责 NoteStructure 的各种操作：
 * - 目录扫描 → NoteNode 树
 * - JSON 文件读 / 写
 * - NoteNode 树 ↔ QJsonObject
 * - NoteNode 树 → QStandardItemModel (用于 TreeView)
 *
 * 不负责 HTTP、不负责 token/baseUrl。
 */
class NoteStructureManager : public QObject
{
    Q_OBJECT
public:
    explicit NoteStructureManager(QObject* parent = nullptr);

    // ========== JSON 读写 ==========
    std::unique_ptr<NoteNode> loadFromJsonFile(const QString& filePath,
        int& maxIdOut);
    void saveToJsonFile(const QString& filePath,
        const NoteNode* root);

    QJsonObject toJson(const NoteNode* node) const;

    // ========== 扫描目录并复用旧 ID ==========
    std::unique_ptr<NoteNode> updateStructureFromDirAndJson(
        const QString& jsonFile,
        const QString& rootDir,
        int& nextIdOut);

    std::unique_ptr<NoteNode> buildFromDirectory(
        const QString& rootDir,
        int& nextId,
        const QHash<QString, IdInfo>* existingIndex = nullptr,
        const QString& parentPath = QString());

    // ========== 构造 TreeView 模型 ==========
    QStandardItemModel* createTreeModel(const NoteNode* root,
        QObject* parent = nullptr) const;

    // 如有需要，你也可以在外部直接用 fromJson + buildPathIndex
    std::unique_ptr<NoteNode> fromJson(
        const QJsonObject& obj,
        int& maxIdInOut,
        const QString& parentPath = QString());

    // 写入remotenoteId
    bool setRemoteNoteIdByAbsolutePath(NoteNode* node,
        const QString& absPath,
        const int remoteId);

    void buildPathIndex(const NoteNode* node,
        QHash<QString, IdInfo>& index,
        const QString& parentPath = QString()) const;

    QString generateId(int& nextId) const;

};
