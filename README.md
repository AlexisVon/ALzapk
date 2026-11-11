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
tar -xzf alzapk-2.0.0-linux-x64.tar.gz && cd alzapk-2.0.0-linux-x64
./lza-cli a -c out.lza README.md src/
./lza-cli l out.lza
```

### 从源码构建

依赖：C++17 编译器；CMake ≥ 3.19；构建 `lza-ui` 需要 Qt6 ≥ 6.5（Core, Widgets）。

`lza-cli` 静态链接 AlxLib。先从 [AlxLib v1.6.1](https://github.com/AlexisVon/AlxLib/releases/tag/v1.6.1) 下载 Linux slim 包，校验后解包到 `lib/alxlib/`：

```bash
P=AlxLib-1.6.1-linux-x64-g845b899-slim.tar.gz
curl -L -o "$P" "https://github.com/AlexisVon/AlxLib/releases/download/v1.6.1/$P"
curl -L -o "$P.sha256" "https://github.com/AlexisVon/AlxLib/releases/download/v1.6.1/$P.sha256"
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
  -p<password> 启用加密（a）或解密（x/l）；默认 v2 认证格式
  -mv1         加密使用旧版 v1 格式（配合 -p）
  -mv2         加密使用认证 v2 格式（配合 -p，默认）
  -k<N>        v2 密钥派生轮数，1..16777216（默认 4194304）
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

`l` 会显示归档类型（v1/v2、是否压缩/加密）以及 v2 归档的元数据（`app`、`version`）；`x` 在开始前打印同样的类型行。

## AlxLib 依赖

`lib/alxlib/` 存放 AlxLib 发行包（当前 1.6.1）。升级步骤：替换压缩包、解包、全量重建。Linux 静态链接使用包内 `lib/libalxlib.a`。

## MSVC

源码保持 MSVC 兼容：路径一律走 `alx::file_info`（本地字符集，与 AlxLib 相同），不依赖 POSIX API，`printf` 只使用 UCRT 支持的 C99 格式。

Visual Studio 2022（x64）用 `msvc/ALzapk.slnx`：

- `lza-core`、`lza-cli`、`lza-ui` 三个工程；`lza-ui` 需要 Qt VS Tools 与 Qt 6.8.3 MSVC 2022 64-bit（`core;gui;widgets`）。
- 把 AlxLib 的 Windows 发行包放到 `lib/alxlib/`（`lib/*.lib` 供链接）；工程会把 `AlxBase.dll`/`AlxCore.dll` 复制到 `bin/`。
- Qt DLL 用 `windeployqt` 部署，或把 Qt 的 `bin` 加入 `PATH`。

CMake 也支持 MSVC：`cmake -B build -G "Visual Studio 17 2022" -A x64`；编译带 `/utf-8`，构建后同样复制 AlxLib DLL。`lza-cli -v` 输出 `2.0.0`。

## 安全说明

带密码新建归档默认使用 v2 认证格式：每个归档生成随机盐，用 PBKDF2-HMAC-SHA256（默认 4 Mi 轮）派生密钥，数据块与索引均由 AES-256-GCM 认证；错密码或被篡改会在打开/解压时报错。`-mv1` 保留旧的 v1 格式（`SHA-256(password)` 直接作密钥、无盐、无认证），仅用于兼容旧工具。

CLI 的密码仍会出现在命令行参数与进程列表中；需要隐藏密码时请使用 UI。弱口令仍是主要风险——PBKDF2 只增加每次猜测的成本，不增加熵。

## 相关项目

- [AlxLib](https://github.com/AlexisVon/AlxLib) — 底层 C++ 库与 `fpacker` 归档引擎

## 许可

Apache License 2.0. Copyright (c) 2026 AlexisVon.

第三方组件与许可全文见 [THIRD-PARTY.md](THIRD-PARTY.md) 与 [`licenses/`](licenses/)。
贡献前请读 [CONTRIBUTING.md](CONTRIBUTING.md) 并签 DCO。
