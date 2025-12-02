package com.riptidez.notebackend.note.controller;

import com.riptidez.notebackend.auth.entity.User;
import com.riptidez.notebackend.auth.service.AuthService;
import com.riptidez.notebackend.note.entity.Note;
import com.riptidez.notebackend.note.entity.NoteHistory;
import com.riptidez.notebackend.note.service.NoteHistoryService;
import com.riptidez.notebackend.note.service.NoteService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@RequestMapping("/notes")
public class NoteController {
    @Autowired
    private NoteService noteService;

    @Autowired
    private NoteHistoryService noteHistoryService;

    @Autowired
    private AuthService authService;

    @PostMapping
    public Note createNote() {
        User user = authService.getCurrentUser();
        return noteService.createEmptyNote(user.getId());
    }

    @GetMapping
    public List<Note> listNotes() {
        User user = authService.getCurrentUser();
        return noteService.listNotes(user.getId());
    }

    @GetMapping("/{noteId}")
    public NoteWithContent getNote(@PathVariable Long noteId) {
        User user = authService.getCurrentUser();
        return noteService.getNoteWithCurrentContent(noteId, user.getId());
    }

    @PutMapping("/{noteId}")
    public NoteHistory updateNote(@PathVariable Long noteId,
                                  @RequestBody UpdateNoteRequest req) {
        User user = authService.getCurrentUser();
        return noteService.updateNote(
                noteId, user.getId(), req.getContent(), req.getChangeSummary());
    }

    @GetMapping("/{noteId}/histories")
    public List<NoteHistory> listHistories(@PathVariable Long noteId) {
        User user = authService.getCurrentUser();
        return noteHistoryService.listHistories(noteId, user.getId());
    }

    @GetMapping("/{noteId}/histories/{version}")
    public NoteHistory getHistory(@PathVariable Long noteId,
                                  @PathVariable Long version) {
        User user = authService.getCurrentUser();
        return noteHistoryService.getHistory(noteId, version, user.getId());
    }

    @PostMapping("/{noteId}/histories/{version}/rollback")
    public NoteHistory rollback(@PathVariable Long noteId,
                                @PathVariable Long version,
                                @RequestBody(required = false) RollbackRequest req) {
        User user = authService.getCurrentUser();
        String changeSummary = (req != null ? req.getChangeSummary() : null);
        return noteHistoryService.rollback(noteId, version, user.getId(), changeSummary);
    }
}
