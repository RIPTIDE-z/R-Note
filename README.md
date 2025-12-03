# R-Note

## 项目说明

- 实践分布式事务控制的项目
    - 后端使用`Spring Boot` + `Druid` + `MyBatis` + `Atomikos` + `MySQL`
    - 前端使用`Qt Widgets`
    - 使用`RESTful API`标准、`APIfox`软件辅助设计接口
- 项目模板为带有简单 **用户注册/登录**，笔记 **历史查看/回滚** 的代码笔记软件

---

### 数据库配置说明

1. 服务端需要创建`auth_db` `note_db`两个数据库
    - 创建脚本位于`Note_back\src\main\resources\sql`
2. 数据库连接所需用户密码配置方式为`application.yml`引入`db_credential.yml`文件，具体信息均在其中配置
    - 请自行根据`db_credential_example.yml`创建`db_credential.yml`并填写信息
    ```yml
    spring:
    config:
        import: "classpath:db_credential.yml"
    ```
---

# Release说明

## Release 0.1

- Release0.1 为框架合作的验证程序，实际运行效果类似学生管理系统

