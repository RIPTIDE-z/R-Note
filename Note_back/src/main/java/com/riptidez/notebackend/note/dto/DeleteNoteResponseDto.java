package com.riptidez.notebackend.note.dto;

public class DeleteNoteResponseDto {
    /** 0表示成功 1表示失败*/
    private Integer code;
    private String message;

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

    public DeleteNoteResponseDto(Integer code, String message) {
        this.code = code;
        this.message = message;
    }

    public static DeleteNoteResponseDto success(String message) {
        return new DeleteNoteResponseDto(0, message);
    }

    public static DeleteNoteResponseDto fail(String message) {
        return new DeleteNoteResponseDto(1, message);
    }
}
