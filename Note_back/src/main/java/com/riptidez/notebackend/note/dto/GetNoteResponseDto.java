package com.riptidez.notebackend.note.dto;

public class GetNoteResponseDto {
    /** 0表示成功 1表示失败*/
    private Integer code;
    private String message;
    private String content;

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

    public String getContent() {
        return content;
    }

    public void setContent(String content) {
        this.content = content;
    }

    public GetNoteResponseDto(Integer code, String message, String content) {
        this.code = code;
        this.message = message;
        this.content = content;
    }

    public static GetNoteResponseDto success(String message, String noteHistory) {
        return new GetNoteResponseDto(0, message, noteHistory);
    }

    public static GetNoteResponseDto fail(String message) {
        return new GetNoteResponseDto(1, message, null);
    }
}
