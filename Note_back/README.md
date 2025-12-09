# R-Note Back-End

## 概述

- 采用`Springboot` + `Mybatis` + `Druid` + `Atomikos` 框架
- 接口设计：
  - `/api`
    - `/auth` 用户账号相关
      - **POST** `/register` 用户注册
      - **POST** `/login` 用户登录
      - **POST** `/logout` 用户登出
    - `/notes` 笔记内容相关
      - **POST** `/{noteId}` 新增 更新 回滚指定笔记
      - **DEL**`/{noteId}` 删除指定笔记
      - **GET**`/{noteId}/{version}` 获取指定笔记的指定版本内容
      - **GET**`/{noteId}/histories` 获取指定笔记的历史列表
    - `/note-structure` 笔记结构相关
      - **GET** 获取用户的笔记结构     
      - **PUT** 更新用户的笔记结构

---

## 代码结构

- `src`
  - `auth`为注册登录相关代码
    - `AuthTokenHelper`用于检验请求给出的token是否有效
    - `SessionService`用于管理会话
  - `note`为笔记相关代码
  - `config`为数据源配置相关代码
  - `exception`为简单包装的异常类，可较简便的抛出message
  - `NoteBackEndApplication`为主程序，基本无需改动
- `resources`
  - `config`
    - `db_credential_example.yml` 数据源连接配置文件示例
    - `mybatis-config.yml` mybatis相关配置文件
    - `logback-spring.yml` logback相关配置文件，但会出现实际启用log4j2的问题
  - `mapper` mybatis的映射文件
  - `sql` 数据库初始化sql文件