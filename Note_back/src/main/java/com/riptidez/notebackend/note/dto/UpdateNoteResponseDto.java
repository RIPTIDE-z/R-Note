package com.riptidez.notebackend.note.dto;

public class UpdateNoteResponseDto {
    private Integer code;
    private String message;
    private Long noteId;

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

    public Long getNoteId() {
        return noteId;
    }

    public void setNoteId(Long noteId) {
        this.noteId = noteId;
    }

    @Override
    public String toString() {
        return "UpdateNoteResponseDto{" +
                "code=" + code +
                ", message='" + message + '\'' +
                ", noteId=" + noteId +
                '}';
    }

    public UpdateNoteResponseDto() {}

    public UpdateNoteResponseDto(Integer code, String message, Long noteId) {
        this.code = code;
        this.message = message;
        this.noteId = noteId;
    }

    public static UpdateNoteResponseDto success(String message, Long noteId) {
        return new UpdateNoteResponseDto(0, message, noteId);
    }

    public static UpdateNoteResponseDto fail(String message, Long noteId) {
        return new UpdateNoteResponseDto(1, message, noteId);
    }
}
