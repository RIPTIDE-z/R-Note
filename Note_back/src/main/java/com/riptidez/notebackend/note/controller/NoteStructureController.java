package com.riptidez.notebackend.note.controller;

import com.riptidez.notebackend.auth.dto.AuthResponseDto;
import com.riptidez.notebackend.auth.service.AuthService;
import com.riptidez.notebackend.auth.service.SessionService;
import com.riptidez.notebackend.note.service.NoteService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/note-structure")
public class NoteStructureController {
    @Autowired
    private NoteService noteService;

    @Autowired
    private SessionService sessionService;

    @Autowired
    private AuthService authService;

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
        Long userId = requireUserId(token);        // 从 token 拿 userId
        String structureJson = authService.getNoteStructure(userId);

        AuthResponseDto resp = new AuthResponseDto();
        resp.setCode(0);
        resp.setMessage("OK");
        resp.setStructure(structureJson);          // 这里就是那个单字符串
        resp.setToken(null);
        return resp;
    }

    @PutMapping("/note-structure")
    public AuthResponseDto updateNoteStructure(@RequestHeader("Auth-Token") String token,
                                               @RequestBody String noteStructureJson) {
        Long userId = requireUserId(token);
        authService.updateNoteStructure(userId, noteStructureJson);

        AuthResponseDto resp = new AuthResponseDto();
        resp.setCode(0);
        resp.setMessage("更新结构成功");
        resp.setStructure(null); // 更新成功不一定要回结构
        resp.setToken(null);
        return resp;
    }

}
