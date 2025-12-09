package com.riptidez.notebackend.note.service;

public interface NoteStructureService {
    /**
     * 获取指定用户的笔记结构
     * @param userId
     * @return
     */
    String getNoteStructure(Long userId);

    /**
     * 更新指定用户的笔记结构
     * @param userId
     * @param noteStructureJson 传入的笔记结构
     */
    void updateNoteStructure(Long userId, String noteStructureJson);
}
