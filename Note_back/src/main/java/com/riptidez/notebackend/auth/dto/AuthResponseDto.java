package com.riptidez.notebackend.auth.dto;

import com.fasterxml.jackson.annotation.JsonRawValue;

/**
 * 用于注册/登录请求的响应体
 */

public class AuthResponseDto {
    /** 标识数字，0代表成功 1代表失败 */
    private Integer code;
    /** 返回的信息 */
    private String message;
    /** Token */
    private String token;
    /** 笔记结构，直接以string格式返回，json解析交给前端 */
    @JsonRawValue
    private String structure;


    public Integer getCode() {
        return code;
    }
    public void setCode(Integer code) {
        this.code = code;
    }

    public String getMessage() { return message;}
    public void setMessage(String message) {
        this.message = message;
    }

    public String getToken() { return token;}
    public void setToken(String token) { this.token = token;}

    public String getStructure() {
        return structure;
    }
    public void setStructure(String structure) {
        this.structure = structure;
    }

    public void setResp(Integer code, String message, String token, String structure) {
        this.code = code;
        this.message = message;
        this.token = token;
        this.structure = structure;
    }

    @Override
    public String toString() {
        return "AuthResponseDto{" +
                "code=" + code +
                ", message='" + message + '\'' +
                ", structure='" + structure + '\'' +
                '}';
    }
}
