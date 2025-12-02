package com.riptidez.notebackend.auth.dto;

public class AuthResponseDto {
    private Integer code;           // "0" 成功, "1" 失败
    private String message;        // 提示信息
    private String structure; // 成功时有，失败时可以为 null

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
