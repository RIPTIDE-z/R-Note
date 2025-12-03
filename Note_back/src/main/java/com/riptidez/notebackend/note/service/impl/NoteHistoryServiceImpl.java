package com.riptidez.notebackend.note.service.impl;

import com.riptidez.notebackend.note.entity.Note;
import com.riptidez.notebackend.note.entity.NoteHistory;
import com.riptidez.notebackend.note.mapper.NoteHistoryMapper;
import com.riptidez.notebackend.note.mapper.NoteMapper;
import com.riptidez.notebackend.note.service.NoteHistoryService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.LocalDateTime;
import java.util.List;

@Service
public class NoteHistoryServiceImpl implements NoteHistoryService {

    @Autowired
    private NoteMapper noteMapper;

    @Autowired
    private NoteHistoryMapper noteHistoryMapper;

    @Override
    public List<NoteHistory> listHistories(Long noteId, Long userId) {
        Note note = noteMapper.findById(noteId);
        if (note == null || !note.getUserId().equals(userId)) {
            throw new RuntimeException("笔记不存在或无权限");
        }
        return noteHistoryMapper.findByNoteId(noteId);
    }

    @Override
    public NoteHistory getHistory(Long noteId, Long version, Long userId) {
        Note note = noteMapper.findById(noteId);
        if (note == null || !note.getUserId().equals(userId)) {
            throw new RuntimeException("笔记不存在或无权限");
        }
        NoteHistory history = noteHistoryMapper.findByNoteIdAndVersion(noteId, version.intValue());
        if (history == null) {
            throw new RuntimeException("历史版本不存在");
        }
        return history;
    }

    @Override
    @Transactional
    public NoteHistory update(Long noteId, Long version, Long userId, String changeSummary) {
        Note note = noteMapper.findById(noteId);
        if (note == null || !note.getUserId().equals(userId)) {
            throw new RuntimeException("笔记不存在或无权限");
        }

        // 找到要回滚到的旧版本
        NoteHistory old = noteHistoryMapper.findByNoteIdAndVersion(noteId, version.intValue());
        if (old == null) {
            throw new RuntimeException("历史版本不存在");
        }

        // 找当前最大 version
        NoteHistory latest = noteHistoryMapper.findLatestByNoteId(noteId);
        int newVersion = (latest == null ? 1 : latest.getVersion() + 1);

        // 用旧内容创建一个新版本
        NoteHistory newHistory = new NoteHistory();
        newHistory.setNoteId(noteId);
        newHistory.setVersion(newVersion);
        newHistory.setCreatedTime(LocalDateTime.now());
        newHistory.setContent(old.getContent());
        newHistory.setChangeSummary(
                changeSummary != null ? changeSummary : ("回滚到版本 " + version)
        );
        noteHistoryMapper.insert(newHistory);

        // 更新 Note.current_history_id
        noteMapper.updateCurrentHistoryId(noteId, newHistory.getId());

        return newHistory;
    }
}
