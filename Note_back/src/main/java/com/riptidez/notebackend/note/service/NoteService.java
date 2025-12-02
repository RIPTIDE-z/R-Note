package com.riptidez.notebackend.note.service;

import com.riptidez.notebackend.note.entity.Note;
import com.riptidez.notebackend.note.entity.NoteHistory;

import java.util.List;

public interface NoteService {
    Note createEmptyNote(Long userId);

    List<Note> listNotes(Long userId);

    // TODO:返回当前内容 + 元数据
    // NoteWithContent getNoteWithCurrentContent(Long noteId, Long userId);

    // 更新笔记 = 新增历史版本 + 更新 current_history_id
    NoteHistory updateNote(Long noteId, Long userId, String content, String changeSummary);
}
