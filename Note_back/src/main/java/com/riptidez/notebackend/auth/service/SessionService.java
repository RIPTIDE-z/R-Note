package com.riptidez.notebackend.auth.service;

/**
 * 使用 token 维护登录会话：
 * - createSession：登录成功后为 userId 生成一个 token
 * - getUserIdByToken：根据 token 找到对应的 userId
 * - removeSession：登出时删除这个 token
 *
 * 当前用内存 Map 实现
 */
public interface SessionService {

    /**
     * 创建一个新的会话：
     * 为指定 userId 生成一个随机 token，并保存映射关系。
     *
     * @param userId 用户 ID（auth_db.User.id）
     * @return 生成的 token 字符串
     */
    String createSession(Long userId);

    /**
     * 根据 token 查找对应的 userId。
     *
     * @param token 前端请求头传来的 token
     * @return 用户 ID，如果 token 无效或不存在，则返回 null
     */
    Long getUserIdByToken(String token);

    /**
     * 删除一个会话：让这个 token 失效。
     *
     * @param token 要删除的 token
     */
    void removeSession(String token);
}
