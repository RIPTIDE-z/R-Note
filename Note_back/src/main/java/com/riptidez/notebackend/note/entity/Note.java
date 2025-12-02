package com.riptidez.notebackend.note.entity;

public class Note {
    private Long id;               // Note.id
    private Long userId;           // Note.user_id（指向 User）
    private Long currentHistoryId; // Note.current_history_id（指向 NoteHistory）

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

    public Long getCurrentHistoryId() {
        return currentHistoryId;
    }

    public void setCurrentHistoryId(Long currentHistoryId) {
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
