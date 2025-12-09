package com.riptidez.notebackend.auth.dto;
/**
 * 用于注册/登录请求的请求体
 */
public class AuthRequestDto {
    /** 用户名 */
    private String username;
    /** 用户密码，前端传入已加密的hash值 */
    private String passwordHash;

    public String getUsername() {
        return username;
    }

    public String getPasswordHash() {
        return passwordHash;
    }
}
