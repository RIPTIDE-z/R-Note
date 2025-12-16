package com.riptidez.notebackend.note.service.impl;

import com.riptidez.notebackend.exception.ExceptionWithMessage;
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
    private NoteHistoryMapper noteHistoryMapper;

    @Autowired
    private NoteMapper noteMapper;

    @Override
    @Transactional
    public List<NoteHistory> listHistories(Long noteId, Long userId) {
        log.info("尝试获取用户{}的笔记{}的历史列表", userId, noteId);
        Note note = noteMapper.findById(noteId);
        if (note == null || !note.getUserId().equals(userId)) {
            log.info("获取用户{}的笔记{}的历史列表失败", userId, noteId);
            throw new ExceptionWithMessage("笔记不存在或无权限");
        }
        log.info("成功获取用户{}的笔记{}的历史列表", userId, noteId);
        return noteHistoryMapper.getNoteHistoryListByNoteId(noteId);
    }

    @Override
    @Transactional
    public NoteHistory getHistory(Long noteId, Integer version, Long userId) {
        log.info("尝试获取用户{}的笔记{}的{}版本", userId, noteId, version);
        Note note = noteMapper.findById(noteId);
        if (note == null || !note.getUserId().equals(userId)) {
            log.info("用户{}的笔记{}不存在或无权限", userId, noteId);
            throw new ExceptionWithMessage("笔记不存在或无权限");
        }
        log.info("成功获取用户{}的笔记{}，尝试获取{}版本", userId, noteId, version);
        NoteHistory history = new NoteHistory();
        if(version > 0){
            history = noteHistoryMapper.getNoteHistoryByNoteIdAndVersion(noteId, version.intValue());
        } else {
            history = noteHistoryMapper.getLatestNoteHistoryByNoteId(noteId);
        }

        if (history == null) {
            log.info("获取用户{}的笔记{}的{}版本失败", userId, noteId, version);
            throw new ExceptionWithMessage("历史版本不存在");
        }
        log.info("成功获取用户{}的笔记{}的{}版本", userId, noteId, version);
        return history;
    }

    @Override
    @Transactional
    public NoteHistory createNote(Long userId,
                                   String content,
                                   String changeSummary) {
        log.info("尝试为用户{}创建新笔记", userId);
        if (userId == null) {
            log.info("userId为空，创建失败");
            throw new ExceptionWithMessage("userId 不能为空");
        }
        if (content == null) {
            log.info("笔记的内容为空，创建失败", userId);
            throw new ExceptionWithMessage("新建笔记 content 不能为空");
        }

        // 1. 新建 Note
        Note note = new Note();
        note.setUserId(userId);

        noteMapper.insert(note);
        log.info("用户{}的新笔记插入成功，尝试创建第一条历史", userId);

        // 2. 创建第一条历史记录 version=1
        NoteHistory history = new NoteHistory();
        history.setNoteId(note.getId());
        history.setVersion(1);
        history.setCreatedTime(LocalDateTime.now());
        history.setContent(content);
        history.setChangeSummary(
                (changeSummary != null && !changeSummary.isEmpty())
                        ? changeSummary
                        : "note init"
        );
        int rows_1 = noteHistoryMapper.insert(history);
        if (rows_1 == 0) {
            log.info("为用户{}新建的笔记{}创建历史失败，无法插入历史", userId, note.getId());
            throw new ExceptionWithMessage("插入历史失败");
        }
        log.info("为用户{}新建的笔记{}创建历史成功", userId, note.getId());

        // 3. 更新 Note.current_history_id
        int rows_2 = noteMapper.updateCurrentHistoryId(note.getId(), history.getId());
        if  (rows_2 != 1) {
            log.info("为用户{}新建的笔记{}创建历史失败，无法更新当前历史版本", userId, note.getId());
            throw new ExceptionWithMessage("插入历史失败");
        }
        log.info("为用户{}新建的笔记{}更新CurrentHistoryId成功", userId, note.getId());

        return history;
    }

    @Override
    @Transactional
    public void updateNote(Long noteId,
                           Long userId,
                           String content,
                           String changeSummary) {
        log.info("尝试更新用户{}的笔记{}", userId, noteId);
        if (content == null) {
            log.info("更新用户{}的笔记{}失败，笔记内容为空", userId, noteId);
            throw new ExceptionWithMessage("更新笔记 content 不能为空");
        }

        Note note = noteMapper.findById(noteId);
        if (note == null || !note.getUserId().equals(userId)) {
            log.info("更新用户{}的笔记{}失败，笔记不存在或者无权限", userId, noteId);
            throw new ExceptionWithMessage("笔记不存在或无权限");
        }
        log.info("已找到用户{}的笔记{}，尝试找寻当前笔记最新版本", userId, noteId);

        // 找当前最大 version
        NoteHistory latest = noteHistoryMapper.getLatestNoteHistoryByNoteId(noteId);
        if (latest == null) {
            log.info("更新用户{}的笔记{}失败，无法找到当前笔记最新版本", userId, noteId);
            throw new ExceptionWithMessage("无法找到当前笔记的最新版本");
        }
        int newVersion = (latest == null ? 1 : latest.getVersion() + 1);

        log.info("已找到用户{}的笔记{}的最新版本，尝试创建新版本", userId, noteId);
        NoteHistory newHistory = new NoteHistory();
        newHistory.setNoteId(noteId);
        newHistory.setVersion(newVersion);
        newHistory.setCreatedTime(LocalDateTime.now());
        newHistory.setContent(content);
        newHistory.setChangeSummary(
                (changeSummary != null && !changeSummary.isEmpty())
                        ? changeSummary
                        : ("更新到版本 " + newVersion)
        );
        log.info("成功为用户{}的笔记{}创建新历史，尝试插入数据", userId, noteId);
        int rows_1 = noteHistoryMapper.insert(newHistory);
        if (rows_1 == 0) {
            log.info("为用户{}的笔记{}创建新历史失败，无法插入历史", userId, note.getId());
            throw new ExceptionWithMessage("更新笔记失败");
        }

        // 更新 Note.current_history_id
        int rows_2 = noteMapper.updateCurrentHistoryId(noteId, newHistory.getId());
        if  (rows_2 != 1) {
            log.info("为用户{}的笔记{}创建新历史失败，无法更新当前历史版本", userId, note.getId());
            throw new ExceptionWithMessage("更新笔记失败");
        }

        log.info("成功更新用户{}的笔记{}", userId, noteId);

    }

    @Override
    public void rollbackNote(Long noteId,
                             Integer targetVersion,
                             Long userId,
                             String changeSummary) {
        log.info("尝试回滚用户{}的笔记{}到版本{}", userId, noteId, targetVersion);
        if (noteId == null || targetVersion == null) {
            log.info("回滚用户{}的笔记{}到版本{}失败，noteId / version 不能为空", userId, noteId, targetVersion);
            throw new ExceptionWithMessage("noteId / version 不能为空");
        }

        Note note = noteMapper.findById(noteId);
        if (note == null || !note.getUserId().equals(userId)) {
            log.info("回滚用户{}的笔记{}到版本{}失败，笔记不存在或无权限", userId, noteId, targetVersion);
            throw new ExceptionWithMessage("笔记不存在或无权限");
        }
        log.info("已找到用户{}的笔记{}，尝试寻找版本{}", userId, noteId, targetVersion);

        // 找到要回滚到的旧版本
        NoteHistory old = noteHistoryMapper.getNoteHistoryByNoteIdAndVersion(noteId, targetVersion);
        if (old == null) {
            log.info("未找到用户{}的笔记{}的版本{}", userId, noteId, targetVersion);
            throw new ExceptionWithMessage("历史版本不存在");
        }
        log.info("已找到用户{}的笔记{}的版本{}，尝试找到当前最新版本", userId, noteId, targetVersion);

        // 找当前最大 version
        NoteHistory latest = noteHistoryMapper.getLatestNoteHistoryByNoteId(noteId);
        if (latest == null) {
            log.info("无法找到用户{}的笔记{}的最新版本", userId, noteId);
            throw new ExceptionWithMessage("无法找到笔记的最新版本");
        }
        int newVersion = (latest == null ? 1 : latest.getVersion() + 1);
        log.info("已找到用户{}的笔记{}的最新版本，尝试创建新版本", userId, noteId);
        // 用旧内容创建一个新版本
        NoteHistory newHistory = new NoteHistory();
        newHistory.setNoteId(noteId);
        newHistory.setVersion(newVersion);
        newHistory.setCreatedTime(LocalDateTime.now());
        newHistory.setContent(old.getContent());
        newHistory.setChangeSummary(
                (changeSummary != null && !changeSummary.isEmpty())
                        ? changeSummary
                        : ("回滚到版本 " + targetVersion)
        );
        log.info("成功为用户{}的笔记{}创建新历史，尝试插入数据", userId, noteId);
        int rows_1 = noteHistoryMapper.insert(newHistory);
        if (rows_1 == 0) {
            log.info("为用户{}的笔记{}创建新历史，无法插入历史", userId, note.getId());
            throw new ExceptionWithMessage("回滚失败");
        }

        // 更新 Note.current_history_id
        int rows_2 = noteMapper.updateCurrentHistoryId(noteId, newHistory.getId());
        if  (rows_2 != 1) {
            log.info("为用户{}的笔记{}创建新历史失败，无法更新当前历史版本", userId, note.getId());
            throw new ExceptionWithMessage("回滚失败");
        }
        log.info("成功回滚用户{}的笔记{}到版本{}，新的版本号为{}", userId, noteId, targetVersion, newHistory.getId());

    }
    @Override
    @Transactional
    public void deleteNote(Long noteId, Long userId) {
        log.info("尝试删除用户{}的笔记{}", userId, noteId);

        if (noteId == null) {
            log.info("删除用户{}的笔记{}失败，noteId 为空", userId, noteId);
            throw new ExceptionWithMessage("noteId 不能为空");
        }

        // 1. 校验笔记归属与权限
        Note note = noteMapper.findById(noteId);
        if (note == null || !note.getUserId().equals(userId)) {
            log.info("删除用户{}的笔记{}失败，笔记不存在或无权限", userId, noteId);
            throw new ExceptionWithMessage("笔记不存在或无权限");
        }

        log.info("已找到用户{}的笔记{}，准备执行删除", userId, noteId);

        // 2. 删除 Note，本身依赖数据库外键级联删除 NoteHistory 记录
        int rows = noteMapper.deleteById(noteId);
        if (rows != 1) {
            log.info("删除用户{}的笔记{}失败，受影响行数={}", userId, noteId, rows);
            throw new ExceptionWithMessage("删除笔记失败");
        }

        log.info("成功删除用户{}的笔记{}", userId, noteId);
    }

}
