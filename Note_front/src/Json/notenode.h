#pragma once
#include <QString>
#include <memory>
#include <optional>
#include <vector>

enum class NoteNodeType { Folder, Note };

// 适用于文件/文件夹的结构体，实际为树结构
struct NoteNode {
    QString id;                          // 节点唯一 ID
    QString name;                        // 文件/文件夹名
    NoteNodeType type;                   // Folder / Note
    std::optional<qint64> remoteNoteId;  // 只有 Note 才有

    // 使用 std::vector + std::unique_ptr 构建树
    std::vector<std::unique_ptr<NoteNode>> children;  // Folder 的子节点数组

    // 从根开始的路径，如 "/root/folder1/a.txt"
    QString fullPath;
    // 绝对路径
    QString absolutePath;

    // 显式禁止拷贝，允许移动，避免以后有人无意中拷贝 NoteNode
    NoteNode() = default;
    NoteNode(const NoteNode&) = delete;
    NoteNode& operator=(const NoteNode&) = delete;
    NoteNode(NoteNode&&) noexcept = default;
    NoteNode& operator=(NoteNode&&) noexcept = default;
};
