package com.riptidez.notebackend.note.controller;

import com.riptidez.notebackend.auth.dto.AuthResponseDto;
import com.riptidez.notebackend.auth.service.AuthService;
import com.riptidez.notebackend.auth.service.SessionService;
import com.riptidez.notebackend.exception.ExceptionWithMessage;
import com.riptidez.notebackend.note.service.NoteService;
import com.riptidez.notebackend.auth.helper.AuthTokenHelper;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

@RestController
public class NoteStructureController {
    @Autowired
    private NoteService noteService;

    @Autowired
    private SessionService sessionService;

    @Autowired
    private AuthService authService;
    @Autowired
    private AuthTokenHelper authTokenHelper;

    /**
     * 从 Auth-Token 取得 userId，如果失败则抛业务异常。
     */
    private Long requireUserId(String token) {
        Long userId = sessionService.getUserIdByToken(token);
        if (userId == null) {
            throw new RuntimeException("未登录或会话已失效");
        }
        return userId;
    }

    @GetMapping("/note-structure")
    public AuthResponseDto getNoteStructure(@RequestHeader("Auth-Token") String token) {
        AuthResponseDto resp = new AuthResponseDto();

        try{
            Long userId = authTokenHelper.requireUserId(token);
            String structureJson = authService.getNoteStructure(userId);

            resp.setCode(0);
            resp.setMessage("成功获取笔记结构");
            resp.setStructure(structureJson);
            resp.setToken(null);

        } catch (ExceptionWithMessage e) {
            resp.setResp(
                    1,
                    e.getMessage(),
                    null,
                    null
            );
        }
        return resp;
    }

    @PutMapping("/note-structure")
    public AuthResponseDto updateNoteStructure(@RequestHeader("Auth-Token") String token,
                                               @RequestBody String noteStructureJson) {
        AuthResponseDto resp = new AuthResponseDto();

        try{
            Long userId = authTokenHelper.requireUserId(token);
            authService.updateNoteStructure(userId, noteStructureJson);

            resp.setCode(0);
            resp.setMessage("更新结构成功");
            resp.setStructure(null); // 更新成功不一定要回结构
            resp.setToken(null);

        } catch (ExceptionWithMessage e) {
            resp.setResp(
                    1,
                    e.getMessage(),
                    null,
                    null
            );
        }
        return resp;
    }
}
