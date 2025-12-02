package com.riptidez.notebackend.note.service;

import com.riptidez.notebackend.note.entity.NoteHistory;

import java.util.List;

public interface NoteHistoryService {
    List<NoteHistory> listHistories(Long noteId, Long userId);

    NoteHistory getHistory(Long noteId, Long version, Long userId);

    NoteHistory rollback(Long noteId, Long version, Long userId, String changeSummary);
}
