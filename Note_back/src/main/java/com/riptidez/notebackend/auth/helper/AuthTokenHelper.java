package com.riptidez.notebackend.auth.helper;

import com.riptidez.notebackend.auth.service.SessionService;
import com.riptidez.notebackend.exception.ExceptionWithMessage;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Component;

@Component
public class AuthTokenHelper {
    private static final Logger log = LoggerFactory.getLogger(AuthTokenHelper.class);

    private final SessionService sessionService;

    public AuthTokenHelper(SessionService sessionService) {
        this.sessionService = sessionService;
    }

    /**
     * 从 Auth-Token 取得 userId，如果失败则抛异常。
     */
    public Long requireUserId(String token) {
        log.info("正在验证token");
        Long userId = sessionService.getUserIdByToken(token);
        if (userId == null) {
            log.error("token验证失败");
            throw new ExceptionWithMessage("未登录或会话已失效");
        }
        return userId;
    }
}
