# =============================================================================
# Compiler Settings and Checks
#
# * C++23 标准与编译器版本门槛
# * 跨平台编译选项
# * 可选：ccache/sccache、unity build、Release 调试信息
# =============================================================================

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_DEBUG_POSTFIX "d")

# cmake-format: off
# 源码里有大量「裸文件名」包含，例如 mimetypes2/mimeutils.cpp 里的
#   #include "mimeutils.h"   （文件实际在 src/utils/）
# 需要把每个目标自身的源码目录/二进制目录加入搜索路径，否则解析不到；
# Qt 的 AUTOGEN（moc/uic 产物）同样依赖二进制目录在 include 路径中。
# cmake-format: on
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(CheckCXXCompilerFlag)

# -----------------------------------------------------------------------------
# 编译器版本门槛（std::expected / C++23 需要较新的前端）
# -----------------------------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12.0)
    message(
      FATAL_ERROR
        "GCC 12.0+ required for C++23 std::expected support. Current: ${CMAKE_CXX_COMPILER_VERSION}"
    )
  endif()

elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 15.0)
    message(
      FATAL_ERROR
        "Clang 15.0+ required for C++23 std::expected support. Current: ${CMAKE_CXX_COMPILER_VERSION}"
    )
  endif()

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.34)
    message(
      FATAL_ERROR
        "MSVC 19.34+ (VS 17.4) required for C++23 std::expected support. Current: ${CMAKE_CXX_COMPILER_VERSION}"
    )
  endif()
endif()

# -----------------------------------------------------------------------------
# 编译器启动器：ccache / sccache（命中缓存时省掉整次编译） 必须在任何 add_library/add_executable 之前设置。
# -----------------------------------------------------------------------------
if(USE_CCACHE AND NOT CMAKE_CXX_COMPILER_LAUNCHER)
  # Windows 上 sccache 对 cl.exe 支持更完整，优先探测
  find_program(COMPILER_LAUNCHER NAMES sccache ccache)
  if(COMPILER_LAUNCHER)
    set(CMAKE_CXX_COMPILER_LAUNCHER
        "${COMPILER_LAUNCHER}"
        CACHE STRING "Compiler launcher (ccache/sccache)")
    message(STATUS "Compiler launcher: ${COMPILER_LAUNCHER}")
  endif()
endif()

# cmake-format: off
# unity build：把多个 .cpp 合成一个翻译单元，显著减少重复的预处理与模板实例化
#   注意：与 SKIP_AUTOMOC 的源文件混用时可能影响 moc 归属，故默认关闭。
# cmake-format: on
if(USE_UNITY_BUILD)
  set(CMAKE_UNITY_BUILD ON)
  set(CMAKE_UNITY_BUILD_BATCH_SIZE 16)
  message(STATUS "Unity build: ON (batch size ${CMAKE_UNITY_BUILD_BATCH_SIZE})")
endif()

# -----------------------------------------------------------------------------
# 项目级编译选项：所有目标自动继承
# -----------------------------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES
                                           "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)

  # 安全检查（仅当编译器支持时添加）
  check_cxx_compiler_flag(-fstack-protector-strong HAS_STACK_PROTECTOR)
  if(HAS_STACK_PROTECTOR)
    add_compile_options(-fstack-protector-strong)
  endif()

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # /utf-8        : 源码含中文注释，缺省代码页会把 UTF-8 当本地代码页解析 /Zc:__cplusplus: 让
  # __cplusplus 反映真实的 -std:c++latest /FS           : 并行编译时串行化 PDB 写入（配合 /Zi 与
  # /MP）
  add_compile_options(/W3 /permissive- /utf-8 /Zc:__cplusplus /FS)

  # cmake-format: off
  # 头文件依赖依赖 cl 的 showIncludes 文本：CMake 在 project() 时探测一次
  # 并写进 CMakeCXXCompiler.cmake。dev.ps1 里设置 VSLANG=1033 让探测和
  # 实际编译都输出英文前缀，否则中文系统上 Ninja 会静默记成 0 依赖。
# cmake-format: on

  # Visual Studio 生成器不会自动并行；Ninja/Makefile 本身已并行，加了会超订
  if(CMAKE_GENERATOR MATCHES "Visual Studio")
    add_compile_options(/MP)
  endif()
endif()

# cmake-format: off
# Release 携带调试信息（与 qmake/CompilerSettings.pri 行为保持一致）
#   排查线上崩溃栈时需要它；不需要时 -DRELEASE_WITH_DEBUGINFO=OFF
# cmake-format: on
if(RELEASE_WITH_DEBUGINFO)
  if(MSVC)
    add_compile_options($<$<CONFIG:Release>:/Zi>)
    add_link_options($<$<CONFIG:Release>:/DEBUG>)
  else()
    add_compile_options($<$<CONFIG:Release>:-g>)
  endif()
endif()
