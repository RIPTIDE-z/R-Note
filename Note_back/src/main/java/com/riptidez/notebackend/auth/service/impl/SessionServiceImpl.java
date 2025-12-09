package com.riptidez.notebackend.auth.service.impl;

import com.riptidez.notebackend.auth.service.SessionService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;

import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;

/**
 * 基于内存 Map 的 Session 实现：
 *
 * - 使用 ConcurrentHashMap 保证并发安全
 * - key: token（随机字符串）
 * - value: userId
 *
 * 注意：程序重启后，所有会话都会失效
 */
@Service
public class SessionServiceImpl implements SessionService {
    private static final Logger log = LoggerFactory.getLogger(SessionServiceImpl.class);

    /**
     * 存储会话的 map:token -> userId
     */
    private final Map<String, Long> sessions = new ConcurrentHashMap<>();

    @Override
    public String createSession(Long userId) {
        if (userId == null) {
            throw new IllegalArgumentException("userId 不能为 null");
        }

        // 生成一个随机 token（简单用 UUID 即可）
        String token = UUID.randomUUID().toString();

        // 保存映射关系
        sessions.put(token, userId);
        log.info("Session成功创建");

        return token;
    }

    @Override
    public Long getUserIdByToken(String token) {
        if (token == null || token.isBlank()) {
            return null;
        }
        return sessions.get(token);
    }

    @Override
    public void removeSession(String token) {
        if (token == null || token.isBlank()) {
            return;
        }
        sessions.remove(token);
        log.info("用户成功登出");
    }
}
