# =============================================================================
# Qt 位置解析 + 全局 Qt 定义
#
# 查找优先级：
#   1. -DQT_ROOT=<dir>
#   2. 环境变量 QT_ROOT
#   3. 调用方已经给出的 CMAKE_PREFIX_PATH / Qt6_DIR（不覆盖）
#   4. 平台默认路径（Qt 在线安装器的默认位置）
#
# 注意：本文件在 project() 之后 include，因此 WIN32/APPLE/UNIX 可用。
# =============================================================================

if(QT_ROOT)
  list(PREPEND CMAKE_PREFIX_PATH "${QT_ROOT}")
  message(STATUS "Qt prefix (QT_ROOT): ${QT_ROOT}")
elseif(DEFINED ENV{QT_ROOT} AND NOT "$ENV{QT_ROOT}" STREQUAL "")
  list(PREPEND CMAKE_PREFIX_PATH "$ENV{QT_ROOT}")
  message(STATUS "Qt prefix ($ENV{QT_ROOT}): $ENV{QT_ROOT}")
elseif(NOT CMAKE_PREFIX_PATH AND NOT Qt6_DIR)
  # 只有调用方完全没给路径时才用默认值，避免悄悄覆盖 -DCMAKE_PREFIX_PATH
  if(WIN32)
    set(QT_DEFAULT_PREFIX "C:/Qt/6.12.0/msvc2022_64")
  elseif(APPLE)
    set(QT_DEFAULT_PREFIX "/opt/Qt/6.12.0/macos")
  elseif(UNIX)
    set(QT_DEFAULT_PREFIX "/opt/Qt/6.12.0/gcc_64")
  else()
    set(QT_DEFAULT_PREFIX "")
  endif()

  if(QT_DEFAULT_PREFIX)
    list(APPEND CMAKE_PREFIX_PATH "${QT_DEFAULT_PREFIX}")
    message(STATUS "Qt prefix (default): ${QT_DEFAULT_PREFIX}")
  endif()
else()
  message(STATUS "Qt prefix (from caller): ${CMAKE_PREFIX_PATH}")
endif()

# -----------------------------------------------------------------------------
# Qt 弃用策略
#   Qt 6 起 QT_DEPRECATED_WARNINGS 已无意义，改用 _UP_TO 系列；
#   与 qmake/CompilerSettings.pri 保持完全一致，避免两条构建链可编译性不同。
#
#   DISABLE = 6.5.0：以 Qt 6.5 LTS 为基线，比它更早被弃用的 API 一律编译报错，
#                    迫使新代码不用「已被 LTS 判死刑」的写法
#   WARN    = 6.12.0：告警上限对齐实际构建版本，6.12 新弃用的 API 也能提前发现
#   编码格式为 0xMMNNPP（QT_VERSION_CHECK）
# -----------------------------------------------------------------------------
set(QT_DEPRECATED_DISABLE_UP_TO
    "0x060500"
    CACHE STRING "禁用此版本之前废弃的 Qt API（默认 6.5.0）")
set(QT_DEPRECATED_WARN_UP_TO
    "0x060C00"
    CACHE STRING "对此版本之前废弃的 Qt API 给出告警（默认 6.12.0）")

add_compile_definitions(
  QT_DISABLE_DEPRECATED_UP_TO=${QT_DEPRECATED_DISABLE_UP_TO}
  QT_WARN_DEPRECATED_UP_TO=${QT_DEPRECATED_WARN_UP_TO})

message(
  STATUS
    "Qt deprecated policy: disable_up_to=${QT_DEPRECATED_DISABLE_UP_TO}, warn_up_to=${QT_DEPRECATED_WARN_UP_TO}"
)
