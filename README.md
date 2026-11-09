# ALzapk — LZA 归档工具

[中文](README.md) | [English](README.en.md)

LZA 是 [AlxLib](https://github.com/AlexisVon/AlxLib) `fpacker` 的归档格式（可选 LZ4 压缩、AES-256 加密）。本仓库提供两个程序：

- **lza-cli** — 命令行工具，除 C/C++ 运行库外无其他依赖
- **lza-ui** — Qt6 图形界面

两者共享 Qt 无关的静态核心库 `lza-core`，并静态链接 AlxLib。

## 项目结构

```
ALzapk/
├── src/
│   ├── core/       # lza-core：fpacker 门面（打包 / 打开 / 导出 / 进度）
│   ├── cli/        # lza-cli
│   └── ui/         # lza-ui（Qt6 Widgets）
├── res/ui/         # qrc、样式、字体、图标
├── lib/alxlib/     # vendored AlxLib 发行包（头文件入库，二进制忽略）
├── msvc/           # Visual Studio 2022 解决方案
└── bin/            # 构建产物
```

## 快速开始

### 下载

Linux x86-64 CLI 包（单个可执行文件 + 许可证与第三方声明）：

```bash
# 从 github.com/AlexisVon/ALzapk/releases 下载，并用同目录的 .sha256 校验
tar -xzf alzapk-1.0.0-linux-x64.tar.gz && cd alzapk-1.0.0-linux-x64
./lza-cli a -c out.lza README.md src/
./lza-cli l out.lza
```

### 从源码构建

依赖：C++17 编译器；CMake ≥ 3.19；构建 `lza-ui` 需要 Qt6 ≥ 6.5（Core, Widgets）。

`lza-cli` 静态链接 AlxLib。先从 [AlxLib v1.5.0](https://github.com/AlexisVon/AlxLib/releases/tag/v1.5.0) 下载 Linux slim 包，校验后解包到 `lib/alxlib/`：

```bash
P=AlxLib-1.5.0-linux-x64-geb3a68e-slim.tar.gz
curl -L -o "$P" "https://github.com/AlexisVon/AlxLib/releases/download/v1.5.0/$P"
curl -L -o "$P.sha256" "https://github.com/AlexisVon/AlxLib/releases/download/v1.5.0/$P.sha256"
sha256sum -c "$P.sha256"
tar -xzf "$P" -C lib/alxlib --strip-components=1

cmake -B build
cmake --build build -j7
```

产物：`bin/lza-cli`、`bin/lza-ui`。可分别用 `-DBUILD_CLI=OFF` / `-DBUILD_UI=OFF` 关闭。

| 选项 | 默认 | 说明 |
|------|------|------|
| `BUILD_CLI` | `ON` | 构建 `lza-cli` |
| `BUILD_UI` | `ON` | 构建 Qt6 图形界面 |
| `LZA_OPT_LEVEL` | `2` | 优化级别（0/1/2/3） |

## CLI 用法

```
lza-cli <命令> [开关] <archive.lza> [<条目>...]

命令：
  a   新建归档并加入文件/目录
  x   解压条目（不给条目则解压全部）
  l   列出归档内容

开关：
  -c           启用 LZ4 压缩
  -p<password> 启用加密（a）或解密（x/l）
  -o<dir>      解压输出目录（默认当前目录）
  -i           忽略不存在的源（继续打包）
  -h, --help   显示帮助
  -v, --version 显示版本
```

示例：

```bash
lza-cli a -c -psecret out.lza README.md src/
lza-cli l -psecret out.lza
lza-cli x -psecret -o/tmp/out out.lza src/main.cpp
lza-cli x -psecret -o/tmp/out out.lza        # 全部解压
```

归档内条目路径以 `/` 分隔，例如 `src/main.cpp`。退出码：0 成功，1 失败，2 用法错误。

## AlxLib 依赖

`lib/alxlib/` 存放 AlxLib 发行包（当前 1.5.0）。升级步骤：替换压缩包、解包、全量重建。Linux 静态链接使用包内 `lib/libalxlib.a`。

## MSVC

源码保持 MSVC 兼容：路径一律走 `alx::file_info`（本地字符集，与 AlxLib 相同），不依赖 POSIX API，`printf` 只使用 UCRT 支持的 C99 格式。

Visual Studio 2022（x64）用 `msvc/ALzapk.slnx`：

- `lza-core`、`lza-cli`、`lza-ui` 三个工程；`lza-ui` 需要 Qt VS Tools 与 Qt 6.8.3 MSVC 2022 64-bit（`core;gui;widgets`）。
- 把 AlxLib 的 Windows 发行包放到 `lib/alxlib/`（`lib/*.lib` 供链接）；工程会把 `AlxBase.dll`/`AlxCore.dll` 复制到 `bin/`。
- Qt DLL 用 `windeployqt` 部署，或把 Qt 的 `bin` 加入 `PATH`。

CMake 也支持 MSVC：`cmake -B build -G "Visual Studio 17 2022" -A x64`；编译带 `/utf-8`，构建后同样复制 AlxLib DLL。`lza-cli -v` 输出 `1.0.0`。

## 安全说明

`lza-cli -p<password>` 的密码直接交给 AlxLib `fpacker` 的 AES-256 接口：不做加盐、口令拉伸（KDF）或密钥分离，且密码会出现在命令行参数与进程列表中。它适合临时防止误读，不适合保护高价值数据；需要强口令保护时，应在密钥输入前端先做 Argon2/scrypt/PBKDF2 等 KDF，或改用外部加密容器。该改动不在本版本范围内。

## 相关项目

- [AlxLib](https://github.com/AlexisVon/AlxLib) — 底层 C++ 库与 `fpacker` 归档引擎

## 许可

Apache License 2.0. Copyright (c) 2026 AlexisVon.

第三方组件与许可全文见 [THIRD-PARTY.md](THIRD-PARTY.md) 与 [`licenses/`](licenses/)。
贡献前请读 [CONTRIBUTING.md](CONTRIBUTING.md) 并签 DCO。
