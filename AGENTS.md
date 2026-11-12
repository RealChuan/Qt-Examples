# AGENTS.md

Qt 6 / C++23 示例仓库：`src/<示例名>/` 每个目录都是一个独立可运行的示例（部分带单元测试）。同一套源码由 CMake（主）与 qmake（备用）两条链维护，脚本、产物路径、测试注册都在仓库内。

## 构建与验证

构建、运行、测试统一走 `scripts/windows/dev.ps1`，参数见 `scripts/windows/README.md`；其它平台按 `.github/workflows/cmake.yml` 用 CMake + CTest。
改完的标准：`-Action test` 通过、`scripts/windows/format.ps1 -Check` 无输出。

## 不可破坏的契约

- **双链同步**：增删源文件时 `CMakeLists.txt` 与同名 `.pro` 必须同时改；新增示例还要注册进 `src/CMakeLists.txt` 与 `src/src.pro`。`*Quick` 仅 CMake 构建，`Chart` 两侧都注释，`WindowsIntegration` 仅 Windows。
- **产物路径**：`binaries/cmake/<arch>/bin|lib/<config>/`、`binaries/qmake/<arch>/bin|lib/<debug|release>/`，packaging 与 CI 依赖该布局。
- **CMake 对外参数**：`-DTARGET_ARCH=<x86_64|arm64|x86|universal>`、`-DVCPKG_TARGET_TRIPLET`。
- **工具链**：CMake ≥ 3.25.1、Qt ≥ 6.7、C++23；GCC ≥ 12、Clang ≥ 15、MSVC ≥ 19.34；Debug 产物带 `d` 后缀。
- **`.ps1` 只写 ASCII**：PowerShell 5.1 按 ANSI 解析无 BOM 文件，中文注释会直接解析失败；中文说明放 Markdown。
- **`README.en.md` 由 CI 从 `README.md` 自动翻译**，不要手改。
- **版本号两处一致**：`packaging/version.txt`（qmake 读）与根 `CMakeLists.txt` 的 `project(VERSION ...)`。

## 代码约定

命名与文件、PIMPL、信号槽、错误处理（`Utils::Result<>`）、导出宏等规则见 `docs/coding-standards.md`，写 C++ 代码前先读。

## 测试

- 测试与模块同目录：`src/<示例>/<示例>_unittest.cc`，`if(BUILD_TESTS)` 门控。
- 不依赖外部服务的用 `qt_add_unit_test(...)` 进 CTest（`src/AsyncLog/CMakeLists.txt`）；需要测试服务器的只编译、不进 CTest（`src/HttpClient/CMakeLists.txt`）。

## 目录

| 路径               | 内容                                                |
| ------------------ | --------------------------------------------------- |
| `src/<示例>/`      | 示例源码：`main.cc`、`CMakeLists.txt`、`.pro`、截图 |
| `cmake/`           | CMake 模块：工具链、输出目录、测试、部署            |
| `qmake/`           | 与 `cmake/` 一一对应的 `.pri`                       |
| `scripts/windows/` | `dev.ps1`、`format.ps1`                             |
| `packaging/`       | `version.txt`、`Enter-VsDevShell.ps1`               |
| `docs/`            | `coding-standards.md`、`superpowers/specs/`         |

## 提交

Conventional Commits：`feat:` / `fix:` / `refactor:` / `docs:` / `chore:`，可带 scope（如 `refactor(build):`），描述用中文。
