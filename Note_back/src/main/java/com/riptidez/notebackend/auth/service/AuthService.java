package com.riptidez.notebackend.auth.service;

import com.riptidez.notebackend.auth.entity.User;
import com.riptidez.notebackend.exception.ExceptionWithMessage;

public interface AuthService {
    User register(String username, String passwordHash) throws ExceptionWithMessage;

    User login(String username, String passwordHash);

    User getCurrentUser();

    String getNoteStructure(Long userId);

    void updateNoteStructure(Long userId, String noteStructureJson);

    User findById(Long id);
}
