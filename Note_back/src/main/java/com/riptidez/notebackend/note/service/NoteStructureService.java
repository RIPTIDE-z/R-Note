package com.riptidez.notebackend.note.service;

public interface NoteStructureService {
    /**
     * 获取指定用户的笔记结构 JSON 字符串（从 User.noteStructure 字段取）
     */
    String getNoteStructure(Long userId);

    /**
     * 更新指定用户的笔记结构 JSON 字符串
     */
    void updateNoteStructure(Long userId, String noteStructureJson);
}
