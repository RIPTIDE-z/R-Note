package com.riptidez.notebackend.note.entity;

/**
 * 笔记元数据实体类
 */

public class Note {
    private Long id;               // Note.id
    private Long userId;           // Note.user_id（指向 User）
    private Integer currentHistoryId; // Note.current_history_id（指向 NoteHistory）

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public Long getUserId() {
        return userId;
    }

    public void setUserId(Long userId) {
        this.userId = userId;
    }

    public Integer getCurrentHistoryId() {
        return currentHistoryId;
    }

    public void setCurrentHistoryId(Integer currentHistoryId) {
        this.currentHistoryId = currentHistoryId;
    }

    @Override
    public String toString() {
        return "Note{" +
                "id=" + id +
                ", userId=" + userId +
                ", currentHistoryId=" + currentHistoryId +
                '}';
    }
}
