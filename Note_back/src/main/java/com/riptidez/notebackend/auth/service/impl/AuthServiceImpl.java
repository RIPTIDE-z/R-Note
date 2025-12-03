package com.riptidez.notebackend.auth.service.impl;

import com.riptidez.notebackend.auth.entity.User;
import com.riptidez.notebackend.auth.mapper.UserMapper;
import com.riptidez.notebackend.auth.service.AuthService;
import com.riptidez.notebackend.exception.ExceptionWithMessage;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

@Service
public class AuthServiceImpl implements AuthService {

    @Autowired
    private UserMapper userMapper;

    public AuthServiceImpl(UserMapper userMapper) {
        this.userMapper = userMapper;
    }

    @Override
    @Transactional
    public User register(String username, String passwordHash) {
        if (userMapper.findHashByUsername(username) != null) {
            throw new ExceptionWithMessage("用户名已存在");
        }

        User user = new User();
        user.setUsername(username);
        user.setPasswordHash(passwordHash);
        user.setNoteStructure("{ }");
        userMapper.insert(user);

        return user;
    }

    @Override
    public User login(String username, String passwordHash) {
        User user = userMapper.findHashByUsername(username);
        if (user == null || !passwordHash.equals(user.getPasswordHash())) {
            throw new ExceptionWithMessage("用户名或密码错误");
        }
        return user;
    }

    @Override
    public User getCurrentUser() {
        //TODO 从 session/token 拿 currentUser
        throw new UnsupportedOperationException("TODO");
    }

    @Override
    public String getNoteStructure(Long userId) {
        User user = userMapper.findById(userId);
        if (user == null) {
            throw new RuntimeException("用户不存在");
        }
        return user.getNoteStructure();
    }

    @Override
    public void updateNoteStructure(Long userId, String noteStructureJson) {
        int n = userMapper.updateNoteStructure(userId, noteStructureJson);
        if (n == 0) {
            throw new RuntimeException("更新笔记结构失败");
        }
    }

    @Override
    public User findById(Long id) {
        return userMapper.findById(id);
    }
}
