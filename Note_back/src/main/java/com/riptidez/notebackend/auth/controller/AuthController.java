package com.riptidez.notebackend.auth.controller;

import com.riptidez.notebackend.auth.dto.AuthRequestDto;
import com.riptidez.notebackend.auth.dto.AuthResponseDto;
import com.riptidez.notebackend.auth.entity.User;
import com.riptidez.notebackend.auth.helper.AuthTokenHelper;
import com.riptidez.notebackend.auth.service.AuthService;
import com.riptidez.notebackend.auth.service.SessionService;
import com.riptidez.notebackend.note.service.NoteService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

/**
 * 用户认证相关接口。
 *
 * Base URL: /auth
 *
 * 接口列表：
 * POST /auth/register
 *   - 功能：用户注册
 *   - 请求体：AuthRequestDto { username, password }
 *   - 响应：AuthResponseDto {
 *       code: 0|1,
 *       message: string,
 *       token: null,
 *       structure: null
 *     }
 *
 * POST /auth/login
 *   - 功能：用户登录
 *   - 请求体：AuthRequestDto { username, password }
 *   - 响应：AuthResponseDto {
 *       code: 0|1,
 *       message: string,
 *       token: string|null,
 *       structure: string|null
 *     }
 *
 * POST /auth/logout
 *   - 功能：用户登出
 *   - 认证：请求头 Auth-Token:
 *   - 响应：AuthResponseDto {
 *       code: 0|1,
 *       message: string,
 *       token: null,
 *       structure: null
 *     }
 * </pre>
 */
@RestController
@RequestMapping("auth")
public class AuthController {

    @Autowired
    private AuthService authService;

    @Autowired
    private SessionService sessionService;

    @Autowired
    private AuthTokenHelper authTokenHelper;

    /** 用户注册接口 */
    @PostMapping("/register")
    public void register(@RequestBody AuthRequestDto req) {
        authService.register(req.getUsername(), req.getPassword());
        // TODO 注册成功/失败的响应
    }

    /** 用户登录接口 */
    @PostMapping("/login")
    public AuthResponseDto login(@RequestBody AuthRequestDto req) {
        User user = authService.login(req.getUsername(), req.getPassword());

        AuthResponseDto resp = new AuthResponseDto();

        if (user == null) {
            resp.setCode(1);
            resp.setMessage("用户名或密码错误");
            resp.setStructure(null);
            resp.setToken(null);
            return resp;
        }

        String token = sessionService.createSession(user.getId());

        resp.setCode(0);
        resp.setMessage("登录成功");
        resp.setStructure(user.getNoteStructure());
        resp.setToken(token);
        return resp;
    }

    /** 用户登出接口 */
    @PostMapping("/logout")
    public AuthResponseDto logout(@RequestHeader("Auth-Token") String token) {
        // 统一用 helper 做校验（无效 token 会抛异常）
        Long userId = authTokenHelper.requireUserId(token);
        // 这里 userId 目前不用，但校验一下是统一做法
        sessionService.removeSession(token);

        AuthResponseDto resp = new AuthResponseDto();
        resp.setCode(0);
        resp.setMessage("登出成功");
        resp.setStructure(null);
        resp.setToken(null);
        return resp;
    }
}
