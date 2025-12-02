-- 创建数据库
CREATE
DATABASE IF NOT EXISTS auth_db
  DEFAULT CHARACTER SET utf8mb4
  COLLATE utf8mb4_unicode_ci;

USE
auth_db;

-- 用户表：User
CREATE TABLE `User`
(
    -- 用户ID：主键，自增，非空
    id             BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '用户ID，主键',

    -- 用户名称：非空，唯一
    username       VARCHAR(64)  NOT NULL COMMENT '用户名，唯一',

    -- 用户密码：存hash值，非空
    password_hash  VARCHAR(255) NOT NULL COMMENT '密码哈希',

    -- 用户笔记结构：非空，这里用 JSON 存笔记结构
    note_structure JSON         NOT NULL COMMENT '用户笔记结构',

    -- 主键约束
    PRIMARY KEY (id),

    -- 用户名唯一约束
    UNIQUE KEY uq_user_username (username)
) ENGINE=InnoDB;