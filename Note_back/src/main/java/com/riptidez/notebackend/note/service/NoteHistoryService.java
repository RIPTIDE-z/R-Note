package com.riptidez.notebackend.note.service;

import com.riptidez.notebackend.note.entity.NoteHistory;

import java.util.List;

public interface NoteHistoryService {
    List<NoteHistory> listHistories(Long noteId, Long userId);

    NoteHistory getHistory(Long noteId, Long version, Long userId);

    /** code = 2：新建 */
    NoteHistory createNote(Long userId,
                           String content,
                           String changeSummary);

    /** code = 0：更新 */
    NoteHistory updateNote(Long noteId,
                           Long userId,
                           String content,
                           String changeSummary);

    /** code = 1：回滚 */
    NoteHistory rollbackNote(Long noteId,
                             Integer targetVersion,
                             Long userId,
                             String changeSummary);
}
