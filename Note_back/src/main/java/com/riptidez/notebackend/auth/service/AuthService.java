package com.riptidez.notebackend.auth.service;

import com.riptidez.notebackend.auth.entity.User;

public interface AuthService {
    /**
     * 通过用户名和密码哈希注册用户，前端已经对密码进行sha256加密
     * @param username
     * @param passwordHash
     * @return
     */
    User register(String username, String passwordHash);

    /**
     * 通过用户名和密码哈希登录用户
     * @param username
     * @param passwordHash
     * @return
     */
    User login(String username, String passwordHash);
}
