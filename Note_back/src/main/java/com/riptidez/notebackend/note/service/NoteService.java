package com.riptidez.notebackend.note.service;

import com.riptidez.notebackend.note.entity.NoteHistory;

import java.util.List;

public interface NoteService {
    /**
     * 获取不带内容的笔记历史列表
     *
     * @param noteId 笔记 id
     * @param userId 用户 id
     * @return 历史记录列表（不含 content）
     */
    List<NoteHistory> listHistories(Long noteId, Long userId);

    /**
     * 获取指定笔记的指定历史版本
     *
     * @param noteId  笔记 id
     * @param version 版本号
     * @param userId  用户 id
     * @return 指定版本的历史记录
     */
    NoteHistory getHistory(Long noteId,
                           Integer version,
                           Long userId);

    /**
     * 创建新笔记，code = 2
     *
     * @param userId        用户 id
     * @param content       笔记内容
     * @param changeSummary 变更摘要
     * @return 新建笔记对应的第一条历史记录
     */
    NoteHistory createNote(Long userId,
                           String content,
                           String changeSummary);

    /**
     * 更新笔记，code = 0
     *
     * @param noteId        笔记 id
     * @param userId        用户 id
     * @param content       新内容
     * @param changeSummary 变更摘要
     */
    void updateNote(Long noteId,
                    Long userId,
                    String content,
                    String changeSummary);

    /**
     * 回滚笔记，code = 1
     *
     * @param noteId        笔记 id
     * @param targetVersion 目标版本号
     * @param userId        用户 id
     * @param changeSummary 变更摘要
     */
    void rollbackNote(Long noteId,
                      Integer targetVersion,
                      Long userId,
                      String changeSummary);

    /**
     * 删除笔记（完全删除笔记及其所有历史版本）
     *
     * @param noteId 笔记主键 id
     * @param userId 当前用户 id
     */
    void deleteNote(Long noteId,
                    Long userId);
}
