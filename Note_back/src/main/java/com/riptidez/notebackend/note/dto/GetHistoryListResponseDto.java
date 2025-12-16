package com.riptidez.notebackend.note.dto;

import com.riptidez.notebackend.note.entity.NoteHistory;

import java.util.List;

public class GetHistoryListResponseDto {
    /** 0表示成功 1表示失败*/
    private Integer code;
    private String message;
    /** 笔记历史列表，不带content */
    private List<NoteHistory> noteHistoryList;

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

    public List<NoteHistory> getNoteHistoryList() {
        return noteHistoryList;
    }

    public void setNoteHistoryList(List<NoteHistory> noteHistoryList) {
        this.noteHistoryList = noteHistoryList;
    }

    public GetHistoryListResponseDto(Integer code, String message, List<NoteHistory> noteHistoryList) {
        this.code = code;
        this.message = message;
        this.noteHistoryList = noteHistoryList;
    }

    public static GetHistoryListResponseDto success(String message, List<NoteHistory> noteHistoryList) {
        return new GetHistoryListResponseDto(0, message, noteHistoryList);
    }

    public static GetHistoryListResponseDto fail(String message, List<NoteHistory> noteHistoryList) {
        return new GetHistoryListResponseDto(1, message, null);
    }
}
