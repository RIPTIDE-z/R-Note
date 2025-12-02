package com.riptidez.notebackend.note.service.impl;

import com.riptidez.notebackend.note.entity.Note;
import com.riptidez.notebackend.note.entity.NoteHistory;
import com.riptidez.notebackend.note.mapper.NoteHistoryMapper;
import com.riptidez.notebackend.note.mapper.NoteMapper;
import com.riptidez.notebackend.note.service.NoteService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.LocalDateTime;
import java.util.List;

@Service
public class NoteServiceImpl implements NoteService {
    @Autowired
    private NoteMapper noteMapper;

    @Autowired
    private NoteHistoryMapper noteHistoryMapper;

    @Override
    @Transactional
    public Note createEmptyNote(Long userId) {
        // 1. 创建 Note
        Note note = new Note();
        note.setUserId(userId);
        noteMapper.insert(note);

        // 2. 创建首个历史版本 version=1，content 为空字符串
        NoteHistory h = new NoteHistory();
        h.setNoteId(note.getId());
        h.setVersion(1);
        h.setCreatedTime(LocalDateTime.now());
        h.setContent("");
        h.setChangeSummary("创建空白笔记");
        noteHistoryMapper.insert(h);

        // 3. 更新 Note.current_history_id
        noteMapper.updateCurrentHistoryId(note.getId(), h.getId());

        return note;
    }

    @Override
    public List<Note> listNotes(Long userId) {
        return noteMapper.findByUserId(userId);
    }


    @Override
    @Transactional
    public NoteHistory updateNote(Long noteId, Long userId, String content, String changeSummary) {
        Note note = noteMapper.findById(noteId);
        if (note == null || !note.getUserId().equals(userId)) {
            throw new RuntimeException("笔记不存在或无权限");
        }

        // 查当前最大 version
        NoteHistory latest = noteHistoryMapper.findLatestByNoteId(noteId);
        int newVersion = (latest == null ? 1 : latest.getVersion() + 1);

        // 新增历史版本
        NoteHistory history = new NoteHistory();
        history.setNoteId(noteId);
        history.setVersion(newVersion);
        history.setCreatedTime(LocalDateTime.now());
        history.setContent(content);
        history.setChangeSummary(changeSummary);
        noteHistoryMapper.insert(history);

        // 更新 Note.current_history_id
        noteMapper.updateCurrentHistoryId(noteId, history.getId());

        return history;
    }
}
