package com.riptidez.notebackend.auth.entity;

/**
 * 用户信息实体类
 */

public class User {
    private Long id;
    private String username;
    private String passwordHash;
    private String noteStructure;

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public String getUsername() {
        return username;
    }

    public void setUsername(String username) {
        this.username = username;
    }

    public String getPasswordHash() {
        return passwordHash;
    }

    public void setPasswordHash(String passwordHash) {
        this.passwordHash = passwordHash;
    }

    public String getNoteStructure() {
        return noteStructure;
    }

    public void setNoteStructure(String noteStructure) {
        this.noteStructure = noteStructure;
    }

    @Override
    public String toString() {
        return "User{" +
                "id=" + id +
                ", username='" + username + '\'' +
                ", passwordHash='" + passwordHash + '\'' +
                ", noteStructure='" + noteStructure + '\'' +
                '}';
    }
}
