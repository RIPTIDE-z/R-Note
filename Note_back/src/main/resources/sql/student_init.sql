CREATE DATABASE IF NOT EXISTS student_db DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

USE student_db;

CREATE TABLE IF NOT EXISTS student
(
    id           INT AUTO_INCREMENT PRIMARY KEY COMMENT 'ID',
    name         VARCHAR(50) NOT NULL COMMENT 'name',
    age          INT         NOT NULL COMMENT 'aga',
    class_name   VARCHAR(50) NOT NULL COMMENT 'class',
    created_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT 'created time',
    updated_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT 'update time'
) ENGINE = InnoDB
  DEFAULT CHARSET = utf8mb4 COMMENT ='student table';

INSERT INTO student (name, age, class_name)
VALUES ('z3', 18, 'compute1'),
       ('l4', 19, 'compute1'),
       ('w5', 18, 'compute2'),
       ('z6', 20, 'software2');