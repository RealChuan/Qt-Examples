cmake_minimum_required(VERSION 3.25.1)

# cmake-format: off
# 构建系统入口
#
# 对外契约（packaging/ 与 CI 依赖，勿随意改动）：
#   -DTARGET_ARCH=<x86_64|arm64|x86|universal>
#   -DVCPKG_TARGET_TRIPLET=<triplet>
#   产物：binaries/<arch-dir>/bin/<config>/ 与 binaries/<arch-dir>/lib/<config>/
# cmake-format: on

# ---- 构建选项 ---------------------------------------------------------------
option(USE_VCPKG "Use vcpkg for dependency management" OFF)
option(USE_CCACHE "探测到 ccache/sccache 时用作编译器启动器" ON)
option(USE_UNITY_BUILD "使用 unity(jumbo) 构建以减少重复解析" OFF)
option(RELEASE_WITH_DEBUGINFO "Release 也生成调试信息（与 qmake 侧一致）" ON)
option(BUILD_TESTS "构建 QTest 单元测试（不安装、不打包）" ON)
option(PROJECT_DEBUG_INFO "打印详细的 CMake 配置信息" OFF)

# ---- 必须在 project() 之前 ------------------------------------------------
# VcpkgToolchain 需要在 project() 触发编译器探测前设置 CMAKE_TOOLCHAIN_FILE；
# macOSDeploymentTarget 需要在编译器初始化前设置 CMAKE_OSX_DEPLOYMENT_TARGET。 这两个文件因此只能用
# CMAKE_HOST_* 判断（那时 WIN32/APPLE/UNIX 尚未定义）。
include(cmake/macOSDeploymentTarget.cmake)

project(
  Qt-Examples
  VERSION 0.0.1
  DESCRIPTION "This is a Qt6 C++ example project for learning and testing."
  HOMEPAGE_URL "https://github.com/RealChuan/Qt-Examples"
  LANGUAGES CXX)

# ---- project() 之后：此时 WIN32 / APPLE / UNIX 才可靠 ----------------------
include(cmake/QtSetup.cmake)
include(cmake/ProjectSettings.cmake)
include(cmake/InstallConfig.cmake)
include(cmake/PlatformLibraries.cmake)
include(cmake/InstallCrashpad.cmake)

find_package(
  Qt6 6.7 REQUIRED
  COMPONENTS Core
             Widgets
             Network
             Concurrent
             Sql
             Quick
             Test)

qt_standard_project_setup()

enable_testing()
include(cmake/QtUnitTests.cmake)

add_subdirectory(src)

include(cmake/CMakeDebugInfo.cmake)
