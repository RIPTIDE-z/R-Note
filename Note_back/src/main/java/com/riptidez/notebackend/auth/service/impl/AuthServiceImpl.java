package com.riptidez.notebackend.auth.service.impl;

import com.riptidez.notebackend.auth.entity.User;
import com.riptidez.notebackend.auth.mapper.UserMapper;
import com.riptidez.notebackend.auth.service.AuthService;
import com.riptidez.notebackend.exception.ExceptionWithMessage;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

@Service
public class AuthServiceImpl implements AuthService {
    private final static Logger log = LoggerFactory.getLogger(AuthServiceImpl.class);

    @Autowired
    private UserMapper userMapper;

    public AuthServiceImpl(UserMapper userMapper) {
        this.userMapper = userMapper;
    }

    @Override
    @Transactional
    public User register(String username, String passwordHash) {
        log.info("尝试注册用户...");
        if (userMapper.findHashByUsername(username) != null) {
            log.warn("用户名已存在，注册失败");
            throw new ExceptionWithMessage("用户名已存在");
        }

        User user = new User();
        user.setUsername(username);
        user.setPasswordHash(passwordHash);
        user.setNoteStructure("{\"version\":1,\"userId\":1,\"root\":{\"id\":\"root\",\"name\":\"root\",\"type\":\"folder\",\"children\":[]}}");
        userMapper.insert(user);
        log.info("注册成功");

        return user;
    }

    @Override
    public User login(String username, String passwordHash) {
        log.info("尝试登录用户{}", username);
        User user = userMapper.findHashByUsername(username);
        if (user == null || !passwordHash.equals(user.getPasswordHash())) {
            log.warn("登录失败，{}不存在", username);
            throw new ExceptionWithMessage("用户名或密码错误");
        }
        log.info("成功登陆{}", username);
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
