package com.riptidez.notebackend.note.service;

import com.riptidez.notebackend.note.entity.Note;
import com.riptidez.notebackend.note.entity.NoteHistory;

import java.util.List;

public interface NoteService {
    Note createEmptyNote(Long userId);

    // TODO:返回当前内容 + 元数据
    NoteHistory getNote(Long noteId, Long userId);

}
