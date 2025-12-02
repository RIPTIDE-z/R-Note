package com.riptidez.notebackend.auth.controller;


import com.riptidez.notebackend.auth.dto.AuthRequestDto;
import com.riptidez.notebackend.auth.dto.AuthResponseDto;
import com.riptidez.notebackend.auth.entity.User;
import com.riptidez.notebackend.auth.service.AuthService;
import com.riptidez.notebackend.note.service.NoteService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("auth")
public class AuthController {

    @Autowired
    private AuthService authService;

    @Autowired
    private NoteService noteService; // 登录时需要同步笔记列表/当前内容的话可用

    @PostMapping("/register")
    public void register(@RequestBody AuthRequestDto req) {
        authService.register(req.getUsername(), req.getPassword());
    }

    @PostMapping("/login")
    public AuthResponseDto login(@RequestBody AuthRequestDto req) {
        // 如果用户名/密码不合法，可以在这里做最基本的校验（可选）
        User user = authService.login(req.getUsername(), req.getPassword());

        AuthResponseDto resp = new AuthResponseDto();
        resp.setCode(0);                       // 约定 0 表示成功
        resp.setMessage("登录成功");
        resp.setStructure(user.getNoteStructure()); // 直接返回 note_structure 字符串

        return resp;
    }

    @PostMapping("/logout")
    public void logout() {
        // TODO: logout逻辑
    }

    @GetMapping("/profile")
    public AuthResponseDto profile() {
        User user = authService.getCurrentUser();

        AuthResponseDto resp = new AuthResponseDto();
        resp.setCode(0);                    // 约定 0 表示成功
        resp.setMessage("OK");              // 或者 "获取用户信息成功"
        resp.setStructure(user.getNoteStructure());  // 直接返回结构 JSON 字符串

        return resp;
    }


    @PutMapping("/note-structure")
    public void updateNoteStructure(@RequestBody String noteStructureJson) {
        User user = authService.getCurrentUser();
        authService.updateNoteStructure(user.getId(), noteStructureJson);
    }
}
