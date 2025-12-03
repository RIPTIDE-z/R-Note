package com.riptidez.notebackend.note.entity;

import java.time.LocalDateTime;

/**
 * 笔记历史实体类
 */

public class NoteHistory {
    private Long id;
    private Long noteId;
    private Integer version;
    private LocalDateTime createdTime;
    private String content;
    private String changeSummary;

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public Long getNoteId() {
        return noteId;
    }

    public void setNoteId(Long noteId) {
        this.noteId = noteId;
    }

    public Integer getVersion() {
        return version;
    }

    public void setVersion(Integer version) {
        this.version = version;
    }

    public LocalDateTime getCreatedTime() {
        return createdTime;
    }

    public void setCreatedTime(LocalDateTime createdTime) {
        this.createdTime = createdTime;
    }

    public String getContent() {
        return content;
    }

    public void setContent(String content) {
        this.content = content;
    }

    public String getChangeSummary() {
        return changeSummary;
    }

    public void setChangeSummary(String changeSummary) {
        this.changeSummary = changeSummary;
    }

    @Override
    public String toString() {
        return "NoteHistory{" +
                "id=" + id +
                ", noteId=" + noteId +
                ", version=" + version +
                ", createdTime=" + createdTime +
                ", content='" + content + '\'' +
                ", changeSummary='" + changeSummary + '\'' +
                '}';
    }
}
