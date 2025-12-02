CREATE DATABASE IF NOT EXISTS log_db DEFAULT CHARSET utf8mb4;

USE log_db;
CREATE TABLE `student_operation_log`
(
    `id`             BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '日志ID',
    `student_id`     BIGINT   NOT NULL COMMENT '学生ID',
    `operation_time` DATETIME NOT NULL COMMENT '操作时间'
) ENGINE = InnoDB
  DEFAULT CHARSET = utf8mb4;
