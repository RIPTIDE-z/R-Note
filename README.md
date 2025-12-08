# R-Note

> - 项目依赖[`qwindowkit`](https://github.com/stdware/qwindowkit)，若要进行前端开发，请使用`--recursive`克隆项目
> - `git clone --recursive https://github.com/RIPTIDE-z/R-Note.git`

## 项目说明

- 实践分布式事务控制的项目
  - 后端使用 `Spring Boot` + `Druid` + `MyBatis` + `Atomikos` + `MySQL`
  - 前端使用 `Qt Widgets` + `qwindowkit`框架
  - 使用 `RESTful API`标准、`APIfox`软件辅助设计接口
- 项目整体为带有简单 **用户注册/登录**，笔记 **历史查看/回滚** 的代码笔记软件

---

### 数据库配置说明

1. 服务端需要创建 `auth_db` `note_db`两个数据库

   - 创建脚本位于 `Note_back\src\main\resources\sql`
2. 数据库连接所需用户密码配置方式为 `application.yml`引入 `db_credential.yml`文件，具体信息均在其中配置

   - **开发环境** :
      - 请自行根据 `db_credential_example.yml`创建 `db_credential.yml`并填写信息
         ```yml
         spring:
            config:
               import: "classpath:db_credential.yml"
         ```
   - **部署**：
      - 使用`optional:file:config/db_credential.yml`的配置选项
      - 在打包后的`jar`文件的同级目录下创建`config`文件夹并在其中放入你配置好的`db_credential.yml`

---

### 前端开发说明

- 项目开发使用`Qt 6.9.3` `VS`+ `Cmake`构建 + `MSVC`工具链 ，请尽量保证环境一致
- 构建预设位于`CMakePresets.json`，默认使用`Ninja`
   - 使用`CMakUserePresets.json`作为用户级配置文件，已给出`CMakUserePresets_example.json`示例
      - 在其中的`QTDIR`中配置你自己Qt库的安装位置，如`"QTDIR": "D:/Qt/6.9.3/msvc2022_64"`
   - 实际构建请选择`Debug` `Release` ， `qt-msvc-debug/release`选项是为了配置基础信息的配置项

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
| 获取笔记结构               | 初步验证 | 12.6 初步验证接口可行性 |
| 更新笔记结构               | 初步验证 | 12.6 初步验证接口可行性 |

---

# Release说明

## Release 0.1

- Release0.1 为框架合作的验证程序，实际运行效果类似学生管理系统

---

## Release 0.2

- Release0.2 为前后端协作的第一个测试版本，验证了注册/登录接口可行性并创建简单前端页面

---

# License说明

- 本项目使用 [MIT协议](./LICENSE.md)
- 项目引用[`qwindowkit`](https://github.com/stdware/qwindowkit),使用 [Apache 2.0 License](https://github.com/stdware/qwindowkit/blob/main/LICENSE)