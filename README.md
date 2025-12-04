# R-Note

## 项目说明

- 实践分布式事务控制的项目
  - 后端使用 `Spring Boot` + `Druid` + `MyBatis` + `Atomikos` + `MySQL`
  - 前端使用 `Qt Widgets`
  - 使用 `RESTful API`标准、`APIfox`软件辅助设计接口
- 项目模板为带有简单 **用户注册/登录**，笔记 **历史查看/回滚** 的代码笔记软件

---

### 数据库配置说明

1. 服务端需要创建 `auth_db` `note_db`两个数据库
   - 创建脚本位于 `Note_back\src\main\resources\sql`
2. 数据库连接所需用户密码配置方式为 `application.yml`引入 `db_credential.yml`文件，具体信息均在其中配置
   - 请自行根据 `db_credential_example.yml`创建 `db_credential.yml`并填写信息

   ```yml
   spring:
   config:
       import: "classpath:db_credential.yml"
   ```

---

# 接口测试记录

| 接口                       |   状态   | 记录                    |
| -------------------------- | :------: | ----------------------- |
| 用户注册                   | 初步验证 | 12.3 初步验证接口可行性 |
| 用户登录                   | 初步验证 | 12.3 初步验证接口可行性 |
| 用户退出登录               | 初步验证 | 12.3 初步验证接口可行性 |
| 创建笔记                   |  待验证  |                         |
| 获取指定笔记的当前版本内容 |  待验证  |                         |
| 获取指定笔记的历史列表     |  待验证  |                         |
| 查看指定版本内容           |  待验证  |                         |
| 更新/回滚笔记              |  待验证  |                         |
| 获取笔记结构               |  待验证  |                         |
| 更新笔记结构               |  待验证  |                         |

---

# Release说明

## Release 0.1

- Release0.1 为框架合作的验证程序，实际运行效果类似学生管理系统

---

## Release 0.2

- Release0.2 为前后端协作的第一个测试版本，验证了注册/登录接口可行性并创建简单前端页面
