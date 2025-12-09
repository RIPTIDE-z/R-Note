# R-Note

> - 项目依赖 [qwindowkit](https://github.com/stdware/qwindowkit)，若要进行前端开发，请使用 `--recursive`克隆项目
> - `git clone --recursive https://github.com/RIPTIDE-z/R-Note.git`

## 项目说明

- 实践分布式事务控制的项目
  - 后端使用 `Spring Boot` + `Druid` + `MyBatis` + `Atomikos` + `MySQL`
  - 前端使用 `Qt Widgets` + `qwindowkit`框架(实现windows无边框设计)
  - 使用 `RESTful API`标准、`APIfox`软件辅助设计接口
- 项目整体为带有简单 **用户注册/登录**，笔记 **历史查看/回滚** 的代码笔记软件

---

### 数据库配置说明

1. 服务端需要安装MySQL创建 `auth_db` `note_db`两个数据库并创建管理员用户(开发环境也可全使用root进行测试)

   - 创建脚本位于 [sql脚本](./Note_back/src/main/resources/sql)
   - 在root运行 `auth_db_init.sql`  `note_db_init.sql`
   - 在root运行 `admin_init.sql`，注意 **配置自己的密码**
     ```sql
     CREATE USER IF NOT EXISTS 'auth_admin'@'localhost'
        IDENTIFIED BY '你的密码';

     CREATE USER IF NOT EXISTS 'note_admin'@'localhost'
        IDENTIFIED BY '你的密码';
     ```
2. 数据库连接所需用户密码配置方式为 `application.yml`引入 `db_credential.yml`文件，具体信息均在其中配置

   - **开发环境** :
     - 请自行根据 `db_credential_example.yml`创建 `db_credential.yml`并填写信息

       ```yml
       spring:
          config:
             import: "classpath:db_credential.yml"
       ```

       ```yml
       auth_db:
          username:
          password:

       note_db:
          username:
          password:
       ```
   - **部署**：
     - 使用 `optional:file:config/db_credential.yml`的配置选项
     - 在打包后的 `jar`文件的同级目录下创建 `config`文件夹并在其中放入你配置好的 `db_credential.yml`

---

### 前端开发说明

- 项目开发使用 `Qt 6.9.3` `VS`+ `Cmake`构建 + `MSVC`工具链 ，请尽量保证环境一致
- 构建预设位于 `CMakePresets.json`，默认使用 `Ninja`
  - 使用 `CMakeUserPresets.json`作为用户级配置文件，已给出 `CMakeUserPresets_example.json`示例
    - 在其中的 `QTDIR`中配置你自己Qt库的安装位置，如 `"QTDIR": "D:/Qt/6.9.3/msvc2022_64"`
  - 要安装则选择带 `（安装）`的选项，默认安装到根目录下的 `bin`文件夹
  - 默认将QWindowkit的库静态编译到程序
    - 使用时要么配置Qt到环境变量要么使用 `Windeployqt`部署程序(用法略)

---

# 接口测试记录

| 接口                       |   状态   | 记录                    |
| -------------------------- | :------: | ----------------------- |
| 用户注册                   | 初步验证 | 12.3 初步验证接口可行性 |
| 用户登录                   | 初步验证 | 12.3 初步验证接口可行性 |
| 用户退出登录               | 初步验证 | 12.3 初步验证接口可行性 |
| ~~创建笔记~~               |  **删除**  | 12.9 接口设计更改，删除   |
| 获取指定笔记的指定版本内容 |  初步验证  |  12.9 初步验证接口可行性  |
|        删除指定笔记        |  待实现  |                         |
| 获取指定笔记的历史列表     |  初步验证  | 12.9 初步验证接口可行性<br>响应返回处理需要进一步细化 |
| 查看指定版本内容           |  初步验证  | 12.9 初步验证接口可行性<br>响应返回处理需要进一步细化  |
| 新建/更新/回滚笔记         |  初步验证 | 12.9 初步验证三种功能可行性  |
| 获取笔记结构               | 初步验证 | 12.6 初步验证接口可行性 |
| 更新笔记结构               | 初步验证 | 12.6 初步验证接口可行性 |

---

# Release说明

## Release 0.1

- 框架合作的验证程序，实际运行效果类似学生管理系统

![](image/release0.1.png)

---

## Release 0.2

- 前后端协作的第一个测试版本，验证了注册/登录接口可行性并创建简单前端页面

---

## Release 0.3

- 第二个测试版本

  - 验证了笔记结构的可行性
  - 建立了配置界面，让配置信息可以主动更改
  - 优化数据库建立和管理逻辑，让linux和windows逻辑统一
- `Ubuntu 22.04` 远端服务器测试：
  ![](image/linux_test.png)

---

# License说明

- 本项目使用 [MIT License](./LICENSE.md)
- 项目引用 [qwindowkit](https://github.com/stdware/qwindowkit),使用 [Apache 2.0 License](https://github.com/stdware/qwindowkit/blob/main/LICENSE)
- md渲染引用 [md4c](https://github.com/mity/md4c),使用 [MIT License](./Note_front/external/Md4c/LICENSE.md)
