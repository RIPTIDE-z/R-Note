package com.riptidez.notebackend.auth.controller;

import com.riptidez.notebackend.auth.dto.AuthRequestDto;
import com.riptidez.notebackend.auth.dto.AuthResponseDto;
import com.riptidez.notebackend.auth.entity.User;
import com.riptidez.notebackend.auth.helper.AuthTokenHelper;
import com.riptidez.notebackend.auth.service.AuthService;
import com.riptidez.notebackend.auth.service.SessionService;
import com.riptidez.notebackend.exception.ExceptionWithMessage;
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
    public AuthResponseDto register(@RequestBody AuthRequestDto req) {
        AuthResponseDto resp = new AuthResponseDto();

        try {
            User user = authService.register(req.getUsername(), req.getPasswordHash());

            resp.setResp(
                    1,
                    "注册成功",
                    null,
                    user.getNoteStructure()
            );

            return resp;
        } catch (ExceptionWithMessage e) {
            resp.setResp(
                    1,
                    e.getMessage(),
                    null,
                    null
            );

            return resp;
        }
    }

    /** 用户登录接口 */
    @PostMapping("/login")
    public AuthResponseDto login(@RequestBody AuthRequestDto req) {
        AuthResponseDto resp = new AuthResponseDto();

        try{
            User user = authService.login(req.getUsername(), req.getPasswordHash());

            /** 创建token */
            String token = sessionService.createSession(user.getId());
            resp.setResp(
                    0,
                    "登录成功",
                    token,
                    user.getNoteStructure()
            );

            return resp;

        } catch (ExceptionWithMessage e) {
            resp.setResp(
                    1,
                    "用户名或密码错误",
                    null,
                    null
            );

            return resp;
        }
    }

    /** 用户登出接口 */
    @PostMapping("/logout")
    public AuthResponseDto logout(@RequestHeader("Auth-Token") String token) {
        AuthResponseDto resp = new AuthResponseDto();

        try {
            Long userId = authTokenHelper.requireUserId(token);
            sessionService.removeSession(token);

            resp.setResp(
                    0,
                    "登出成功",
                    null,
                    null
            );

            return resp;
        } catch (ExceptionWithMessage e){
            resp.setResp(
                    1,
                    e.getMessage(),
                    null,
                    null
            );

            return resp;
        }
    }
}
