package com.riptidez.notebackend.auth.helper;

import com.riptidez.notebackend.auth.service.SessionService;
import org.springframework.stereotype.Component;

@Component
public class AuthTokenHelper {

    private final SessionService sessionService;

    public AuthTokenHelper(SessionService sessionService) {
        this.sessionService = sessionService;
    }

    /**
     * 从 Auth-Token 取得 userId，如果失败则抛异常。
     */
    public Long requireUserId(String token) {
        Long userId = sessionService.getUserIdByToken(token);
        if (userId == null) {
            // 如果你有自定义 BusinessException，就替换成它
            throw new RuntimeException("未登录或会话已失效");
        }
        return userId;
    }
}
