package com.riptidez.notebackend.auth.service;

import com.riptidez.notebackend.auth.entity.User;

public interface AuthService {
    /**
     * 注册：接收的就是前端已经计算好的密码哈希值
     */
    void register(String username, String passwordHash);

    /**
     * 登录：同样接收哈希值，和数据库中的 password_hash 做一致性校验
     */
    User login(String username, String passwordHash);

    /**
     * 获取当前登录用户（从 session/token 中解析）
     */
    User getCurrentUser();

    /**
     * 获取当前用户的笔记结构 JSON
     */
    String getNoteStructure(Long userId);

    /**
     * 更新当前用户的笔记结构 JSON
     */
    void updateNoteStructure(Long userId, String noteStructureJson);
}
