package com.riptidez.notebackend.note.service.impl;

import com.riptidez.notebackend.auth.entity.User;
import com.riptidez.notebackend.auth.mapper.UserMapper;
import com.riptidez.notebackend.auth.service.impl.AuthServiceImpl;
import com.riptidez.notebackend.exception.ExceptionWithMessage;
import com.riptidez.notebackend.note.service.NoteStructureService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

/**
 * 处理笔记结构相关
 */

@Service
public class NoteStructureServiceImpl implements NoteStructureService {
    private final static Logger log = LoggerFactory.getLogger(NoteStructureServiceImpl.class);

    @Autowired
    private UserMapper userMapper;

    @Override
    public String getNoteStructure(Long userId) {
        log.info("尝试获取用户{}的笔记结构", userId);
        User user = userMapper.findById(userId);
        if (user == null) {
            log.info("用户{}不存在", userId);
            throw new ExceptionWithMessage("用户不存在: " + userId);
        }
        log.info("成功获取用户{}的笔记结构", userId);
        return user.getNoteStructure();
    }

    @Override
    @Transactional
    public void updateNoteStructure(Long userId, String noteStructureJson) {
        log.info("尝试更新用户{}的笔记结构", userId);
        int rows = userMapper.updateNoteStructure(userId, noteStructureJson);
        if (rows == 0) {
            throw new ExceptionWithMessage("更新笔记结构失败, userId=" + userId);
        }
        log.info("成功更新用户{}的笔记结构", userId);
    }
}
