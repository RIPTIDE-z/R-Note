package com.riptidez.notebackend.note.service.impl;

import com.riptidez.notebackend.note.entity.Note;
import com.riptidez.notebackend.note.entity.NoteHistory;
import com.riptidez.notebackend.note.mapper.NoteHistoryMapper;
import com.riptidez.notebackend.note.mapper.NoteMapper;
import com.riptidez.notebackend.note.service.NoteService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.LocalDateTime;
import java.util.List;

@Service
public class NoteServiceImpl implements NoteService {
    private final static Logger log = LoggerFactory.getLogger(NoteServiceImpl.class);
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
        log.info("成功创建新笔记");

        return note;
    }

    @Override
    public NoteHistory getNote(Long noteId, Long userId) {

        NoteHistory note =  new NoteHistory();
        note = noteHistoryMapper.findByNoteIdAndVersion(noteId, 1);

        return note;
    }
}
