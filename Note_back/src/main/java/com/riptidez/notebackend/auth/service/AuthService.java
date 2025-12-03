package com.riptidez.notebackend.auth.service;

import com.riptidez.notebackend.auth.entity.User;

public interface AuthService {
    void register(String username, String passwordHash);

    User login(String username, String passwordHash);

    User getCurrentUser();

    String getNoteStructure(Long userId);

    void updateNoteStructure(Long userId, String noteStructureJson);

    User findById(Long id);
}
