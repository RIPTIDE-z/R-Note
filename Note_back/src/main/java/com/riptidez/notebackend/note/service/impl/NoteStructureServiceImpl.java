package com.riptidez.notebackend.note.service.impl;

import com.riptidez.notebackend.auth.entity.User;
import com.riptidez.notebackend.auth.mapper.UserMapper;
import com.riptidez.notebackend.note.service.NoteStructureService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

/**
 * 处理笔记结构相关
 */

@Service
public class NoteStructureServiceImpl implements NoteStructureService {

    @Autowired
    private UserMapper userMapper;

    @Override
    public String getNoteStructure(Long userId) {
        User user = userMapper.findById(userId);
        if (user == null) {
            throw new RuntimeException("用户不存在: " + userId);
        }
        return user.getNoteStructure();
    }

    @Override
    @Transactional
    public void updateNoteStructure(Long userId, String noteStructureJson) {
        int rows = userMapper.updateNoteStructure(userId, noteStructureJson);
        if (rows == 0) {
            throw new RuntimeException("更新笔记结构失败, userId=" + userId);
        }
    }
}
