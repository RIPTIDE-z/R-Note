-- 为两个数据库分别创建管理员用户

-- 为 auth_db 创建管理员用户
CREATE USER IF NOT EXISTS 'auth_admin'@'localhost'
    IDENTIFIED BY 'password';

-- 为 note_db 创建管理员用户
CREATE USER IF NOT EXISTS 'note_admin'@'localhost'
    IDENTIFIED BY 'password';

-- auth_admin授权
GRANT
    SELECT,
    INSERT,
    UPDATE,
    DELETE,
    CREATE,
    DROP,
    INDEX,
    ALTER,
    CREATE TEMPORARY TABLES,
    CREATE VIEW,
    SHOW VIEW,
    CREATE ROUTINE,
    ALTER ROUTINE,
    EXECUTE,
    REFERENCES,
    TRIGGER
    ON auth_db.*
    TO 'auth_admin'@'localhost';

-- note_admin授权
GRANT
    SELECT,
    INSERT,
    UPDATE,
    DELETE,
    CREATE,
    DROP,
    INDEX,
    ALTER,
    CREATE TEMPORARY TABLES,
    CREATE VIEW,
    SHOW VIEW,
    CREATE ROUTINE,
    ALTER ROUTINE,
    EXECUTE,
    REFERENCES,
    TRIGGER
    ON note_db.*
    TO 'note_admin'@'localhost';

FLUSH PRIVILEGES;
