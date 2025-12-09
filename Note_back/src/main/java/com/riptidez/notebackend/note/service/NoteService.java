package com.riptidez.notebackend.note.service;

import com.riptidez.notebackend.note.entity.NoteHistory;

import java.util.List;

public interface NoteService {
    /**
     * 获取不带内容的笔记历史列表
     * @param noteId
     * @param userId
     * @return
     */
    List<NoteHistory> listHistories(Long noteId, Long userId);

    /**
     * 获取笔记的指定历史版本
     * @param noteId
     * @param version
     * @param userId
     * @return
     */
    NoteHistory getHistory(Long noteId, Integer version, Long userId);

    /**
     * 创建新笔记，code = 2
     * @param userId
     * @param content
     * @param changeSummary
     * @return
     */
    NoteHistory createNote(Long userId,
                           String content,
                           String changeSummary);

    /**
     * 更新笔记，code = 0
     *
     * @param noteId
     * @param userId
     * @param content
     * @param changeSummary
     */
    void updateNote(Long noteId,
                    Long userId,
                    String content,
                    String changeSummary);

    /**
     * 回滚笔记，code = 1
     *
     * @param noteId
     * @param targetVersion
     * @param userId
     * @param changeSummary
     */
    void rollbackNote(Long noteId,
                      Integer targetVersion,
                      Long userId,
                      String changeSummary);
}
