package com.riptidez.notebackend.auth.entity;

public class User {
    private Long id;               // auth_db.User.id
    private String username;       // 唯一
    private String passwordHash;   // 密码hash
    private String noteStructure;  // 存 JSON 字符串

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
