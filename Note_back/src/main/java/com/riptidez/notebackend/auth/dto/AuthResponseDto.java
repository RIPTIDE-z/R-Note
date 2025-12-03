package com.riptidez.notebackend.auth.dto;

/**
 * 用于注册/登录请求的响应体
 */

public class AuthResponseDto {
    /** 标识数字，0代表成功 1代表失败 */
    private Integer code;
    /** 返回的信息 */
    private String message;
    /** 笔记结构，直接以string格式返回，json解析交给前端 */
    private String structure;

    public Integer getCode() {
        return code;
    }

    public void setCode(Integer code) {
        this.code = code;
    }

    public String getMessage() {
        return message;
    }

    public void setMessage(String message) {
        this.message = message;
    }

    public String getStructure() {
        return structure;
    }

    public void setStructure(String structure) {
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
