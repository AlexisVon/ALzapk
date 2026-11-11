# ALzapk — The LZA Archiver

[中文](README.md) | [English](README.en.md)

LZA is the archive format of AlxLib's `fpacker` (optional LZ4 compression, AES-256 encryption). This repository provides two programs:

- **lza-cli** — a command-line tool with no dependencies beyond the C/C++ runtime
- **lza-ui** — a Qt6 graphical interface

Both share `lza-core`, a Qt-free static core library, and link AlxLib statically.

## Repository layout

```
ALzapk/
├── src/
│   ├── core/       # lza-core: fpacker facade (pack / open / extract / progress)
│   ├── cli/        # lza-cli
│   └── ui/         # lza-ui (Qt6 Widgets)
├── res/ui/         # qrc, styles, fonts, icons
├── lib/alxlib/     # vendored AlxLib release package (headers tracked, binaries ignored)
├── msvc/           # Visual Studio 2022 solution
└── bin/            # build output
```

## Quickstart

### Download

The Linux x86-64 CLI package (a single executable plus license and third-party notices):

```bash
# Download from github.com/AlexisVon/ALzapk/releases and verify against the .sha256 beside it
tar -xzf alzapk-1.0.0-linux-x64.tar.gz && cd alzapk-1.0.0-linux-x64
./lza-cli a -c out.lza README.md src/
./lza-cli l out.lza
```

### Build from source

Dependencies: a C++17 compiler; CMake ≥ 3.19; Qt6 ≥ 6.5 (Core, Widgets) for `lza-ui`.

`lza-cli` links AlxLib statically. Download the Linux slim package from [AlxLib v1.6.1](https://github.com/AlexisVon/AlxLib/releases/tag/v1.6.1), verify it, and unpack it into `lib/alxlib/`:

```bash
P=AlxLib-1.6.1-linux-x64-g845b899-slim.tar.gz
curl -L -o "$P" "https://github.com/AlexisVon/AlxLib/releases/download/v1.6.1/$P"
curl -L -o "$P.sha256" "https://github.com/AlexisVon/AlxLib/releases/download/v1.6.1/$P.sha256"
sha256sum -c "$P.sha256"
tar -xzf "$P" -C lib/alxlib --strip-components=1

cmake -B build
cmake --build build -j7
```

Outputs: `bin/lza-cli` and `bin/lza-ui`. Use `-DBUILD_CLI=OFF` / `-DBUILD_UI=OFF` to turn either one off.

| Option | Default | Meaning |
|--------|---------|---------|
| `BUILD_CLI` | `ON` | build `lza-cli` |
| `BUILD_UI` | `ON` | build the Qt6 GUI |
| `LZA_OPT_LEVEL` | `2` | optimization level (0/1/2/3) |

## CLI usage

```
lza-cli <command> [switches] <archive.lza> [<item>...]

Commands:
  a   Add files or directories to a new archive
  x   Extract entries (the whole archive when no item is given)
  l   List the archive contents

Switches:
  -c            Compress file data (LZ4)
  -p<password>  Encrypt with a password (v2 authenticated by default)
  -mv1          Use the legacy v1 encryption format (with -p)
  -mv2          Use the authenticated v2 format (with -p, default)
  -k<N>         v2 key-derivation rounds, 1..16777216 (default 4194304)
  -o<dir>       Output directory for x (default: current directory)
  -i            Ignore missing sources when packing
  -h, --help    Show this help
  -v, --version Show version
```

Examples:

```bash
lza-cli a -c -psecret out.lza README.md src/
lza-cli l -psecret out.lza
lza-cli x -psecret -o/tmp/out out.lza src/main.cpp
lza-cli x -psecret -o/tmp/out out.lza        # extract everything
```

Item paths use `/` as the separator, e.g. `src/main.cpp`. Exit codes: 0 success, 1 failure, 2 usage error.

`l` reports the archive type (v1/v2, compressed/encrypted) and the v2 metadata (`app`, `version`); `x` prints the same type line before extracting.

## AlxLib dependency

`lib/alxlib/` holds the AlxLib release package (currently 1.6.1). To upgrade, replace the archives, extract, and rebuild everything. Linux static linking uses `lib/libalxlib.a` from the package.

## MSVC

The sources stay MSVC-compatible: paths go through `alx::file_info` (the local charset model, the same as AlxLib), no POSIX APIs, and only UCRT-supported C99 formats in `printf`.

Visual Studio 2022 (x64) uses `msvc/ALzapk.slnx`:

- Three projects: `lza-core`, `lza-cli`, `lza-ui`. `lza-ui` needs Qt VS Tools and Qt 6.8.3 MSVC 2022 64-bit (`core;gui;widgets`).
- Put the AlxLib Windows package under `lib/alxlib/` (`lib/*.lib` for linking); the projects copy `AlxBase.dll`/`AlxCore.dll` into `bin/`.
- Deploy Qt DLLs with `windeployqt`, or add Qt's `bin` to `PATH`.

CMake also supports MSVC: `cmake -B build -G "Visual Studio 17 2022" -A x64`; it compiles with `/utf-8` and copies the AlxLib DLLs after the build. `lza-cli -v` prints `1.0.0`.

## Security note

New archives with a password use the v2 authenticated format by default: each archive gets a random salt, the key comes from PBKDF2-HMAC-SHA256 (4 Mi rounds by default), and both the data blocks and the index are authenticated with AES-256-GCM; a wrong password or tampering fails at open or extract time. `-mv1` keeps the legacy v1 format (`SHA-256(password)` as the key directly, no salt, no authentication) for compatibility with old tools.

The CLI still exposes the password in the command line and process list; use the UI when that matters. A weak password remains the main risk — PBKDF2 raises the cost of each guess, it adds no entropy.

## Related projects

- [AlxLib](https://github.com/AlexisVon/AlxLib) — the underlying C++ library and the `fpacker` archive engine

## License

Apache License 2.0. Copyright (c) 2026 AlexisVon.

Third-party components and full license texts: see [THIRD-PARTY.md](THIRD-PARTY.md) and [`licenses/`](licenses/).
Read [CONTRIBUTING.md](CONTRIBUTING.md) and sign the DCO before contributing.
