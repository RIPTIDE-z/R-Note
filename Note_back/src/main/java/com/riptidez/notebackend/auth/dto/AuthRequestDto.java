package com.riptidez.notebackend.auth.dto;
/**
 * 用于注册/登录请求的请求体
 */
public class AuthRequestDto {
    /** 用户名 */
    private String username;
    /** 用户密码，前端传入已加密的hash值 */
    private String password;

    public String getUsername() {
        return username;
    }

    public void setUsername(String username) {
        this.username = username;
    }

    public String getPassword() {
        return password;
    }

    public void setPassword(String password) {
        this.password = password;
    }

    @Override
    public String toString() {
        return "LoginRequest{" +
                "username='" + username + '\'' +
                ", password='" + password + '\'' +
                '}';
    }
}
