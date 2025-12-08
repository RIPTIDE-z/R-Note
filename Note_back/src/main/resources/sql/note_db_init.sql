-- 创建note数据库
CREATE
DATABASE IF NOT EXISTS note_db
  DEFAULT CHARACTER SET utf8mb4
  COLLATE utf8mb4_unicode_ci;

USE note_db;

-- 笔记表
CREATE TABLE `note`
(
    -- 笔记ID：主键，自增，非空
    id                 BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '笔记ID，主键',

    -- 所属用户ID：非空，外键到 auth_db.user.id
    user_id            BIGINT UNSIGNED NOT NULL COMMENT '所属用户ID',

    -- 当前历史ID：可以为空（刚创建时可不指向任何历史），外键到 NoteHistory.id
    current_history_id BIGINT UNSIGNED NULL COMMENT '当前历史版本ID',

    PRIMARY KEY (id)
) ENGINE=InnoDB;

-- 笔记历史表
CREATE TABLE `note_history`
(
    -- ID：主键，自增，非空
    id             BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '历史记录ID，主键',

    -- 对应笔记ID：非空，外键到 Note.id
    note_id        BIGINT UNSIGNED NOT NULL COMMENT '对应笔记ID',

    -- 版本号：1,2,3... 递增的整数，非空，>=1
    version        INT UNSIGNED NOT NULL COMMENT '版本号，从1开始递增',

    -- 创建时间：非空
    created_time   DATETIME(6) NOT NULL COMMENT '创建时间',

    -- 笔记内容：非空
    content        MEDIUMTEXT   NOT NULL COMMENT '笔记内容',

    -- 修改说明：非空
    change_summary VARCHAR(255) NOT NULL COMMENT '修改说明',

    PRIMARY KEY (id),

    -- 同一 note_id 下 version 唯一
    UNIQUE KEY uq_notehistory_note_version (note_id, version),

    -- 外键：NoteHistory.note_id -> Note.id
    -- 删除笔记时自动删除历史
    -- 若手动改id会自动同步
    CONSTRAINT fk_notehistory_note
        FOREIGN KEY (note_id)
            REFERENCES `note` (id)
            ON DELETE CASCADE
            ON UPDATE CASCADE,

    -- 可选的版本号合法性检查（>=1）
    CHECK (version >= 1)
) ENGINE=InnoDB;

-- 外键 Note.current_history_id -> NoteHistory.id
-- 若删除的历史刚好是current_history_id，会将其置NULL
-- id同步
ALTER TABLE `note`
    ADD CONSTRAINT fk_note_current_history
        FOREIGN KEY (current_history_id)
            REFERENCES `note_history` (id)
            ON DELETE SET NULL
            ON UPDATE CASCADE;