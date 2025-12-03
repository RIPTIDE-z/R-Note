package com.riptidez.notebackend.note.controller;

import com.riptidez.notebackend.auth.helper.AuthTokenHelper;
import com.riptidez.notebackend.auth.service.SessionService;
import com.riptidez.notebackend.note.dto.UpdateNoteRequestDto;
import com.riptidez.notebackend.note.entity.Note;
import com.riptidez.notebackend.note.entity.NoteHistory;
import com.riptidez.notebackend.note.service.NoteHistoryService;
import com.riptidez.notebackend.note.service.NoteService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import java.util.List;

/**
 * 笔记及历史相关接口。
 *
 * Base URL: /notes
 *
 * 接口列表：
 * <pre>
 * POST /notes
 *   - 功能：为当前登录用户创建一个空白笔记
 *   - 认证：请求头 Auth-Token: string
 *   - 请求体：{}
 *   - 响应：Note {
 *       id: long,
 *       userId: long,
 *       currentHistoryId: long
 *     }
 *
 * GET /notes/{noteId}
 *   - 功能：获取指定笔记的当前内容
 *   - 认证：请求头 Auth-Token: string
 *   - 路径参数：
 *       noteId: long
 *   - 响应：NoteHistory {
 *       id: long,
 *       noteId: long,
 *       version: int,
 *       createdTime: string,
 *       content: string,
 *       changeSummary: string
 *     }
 *
 * GET /notes/{noteId}/histories
 *   - 功能：获取指定笔记的历史版本列表（一般不返回 content）
 *   - 认证：请求头 Auth-Token: string
 *   - 路径参数：
 *       noteId: long
 *   - 响应：List<NoteHistory>，元素形如：
 *     {
 *       id: long,
 *       noteId: long,
 *       version: int,
 *       createdTime: string,
 *       changeSummary: string
 *     }
 *
 * GET /notes/{noteId}/histories/{version}
 *   - 功能：查看指定版本的完整内容
 *   - 认证：请求头 Auth-Token: string
 *   - 路径参数：
 *       noteId: long
 *       version: int
 *   - 响应：NoteHistory {
 *       id: long,
 *       noteId: long,
 *       version: int,
 *       createdTime: string,
 *       content: string,
 *       changeSummary: string
 *     }
 *
 * POST /notes/{noteId}/histories
 *   - 功能：基于指定版本创建一个新的当前版本（更新/回滚）
 *   - 认证：请求头 Auth-Token: string
 *   - 路径参数：
 *       noteId: long
 *       version: int
 *   - 请求体：UpdateNoteRequestDto {
 *       changeSummary: string
 *     }
 *   - 响应：NoteHistory {
 *       id: long,
 *       noteId: long,
 *       version: int,
 *       createdTime: string,
 *       content: string,
 *       changeSummary: string
 *     }
 * </pre>
 */

@RestController
@RequestMapping("/notes")
public class NoteController {

    @Autowired
    private NoteService noteService;

    @Autowired
    private NoteHistoryService noteHistoryService;

    @Autowired
    private AuthTokenHelper authTokenHelper;


    @PostMapping
    public Note createNote(@RequestHeader("Auth-Token") String token) {
        Long userId = authTokenHelper.requireUserId(token);
        return noteService.createEmptyNote(userId);
    }

    @GetMapping("/{noteId}")
    public NoteHistory getNote(@RequestHeader("Auth-Token") String token,
                               @PathVariable Long noteId) {
        Long userId = authTokenHelper.requireUserId(token);
        return noteService.getNote(noteId, userId);
    }

    @GetMapping("/{noteId}/histories")
    public List<NoteHistory> listHistories(@RequestHeader("Auth-Token") String token,
                                           @PathVariable Long noteId) {
        Long userId = authTokenHelper.requireUserId(token);
        return noteHistoryService.listHistories(noteId, userId);
    }

    @GetMapping("/{noteId}/histories/{version}")
    public NoteHistory getHistory(@RequestHeader("Auth-Token") String token,
                                  @PathVariable Long noteId,
                                  @PathVariable Long version) {
        Long userId = authTokenHelper.requireUserId(token);
        return noteHistoryService.getHistory(noteId, version, userId);
    }

    @PostMapping("/{noteId}/histories")
    public NoteHistory updateFromHistory(@RequestHeader("Auth-Token") String token,
                                         @PathVariable Long noteId,
                                         @PathVariable Long version,
                                         @RequestBody(required = false) UpdateNoteRequestDto req) {
        Long userId = authTokenHelper.requireUserId(token);
        String changeSummary = (req != null ? req.getChangeSummary() : null);
        return noteHistoryService.update(noteId, version, userId, changeSummary);
    }
}
