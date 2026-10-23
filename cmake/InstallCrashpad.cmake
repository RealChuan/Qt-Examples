# =============================================================================
# Crashpad Handler 拷贝与安装
#
# 旧实现是在 configure 期用 file(GLOB) + 时间戳判断后 file(INSTALL)：
#   - 只在 configure 时执行，crashpad 更新后不重新 configure 就不会同步
#   - 每次 configure 都要 stat 一遍
# 现在改为构建期的 POST_BUILD 自定义命令，交给构建系统判断新鲜度。
# =============================================================================

# -----------------------------------------------------------------------------
# 定位 crashpad 的 tools 目录（vcpkg 布局：<prefix>/share/crashpad -> <prefix>/tools/crashpad）
# -----------------------------------------------------------------------------
macro(get_crashpad_tools_path output_var)
  if(NOT crashpad_DIR)
    message(FATAL_ERROR "crashpad_DIR is not set")
  endif()

  # 优先按 vcpkg 布局推导：crashpad_DIR = <prefix>/share/crashpad
  get_filename_component(_crashpad_prefix "${crashpad_DIR}/../.." ABSOLUTE)
  set(_crashpad_tools "${_crashpad_prefix}/tools/crashpad")

  if(NOT EXISTS "${_crashpad_tools}")
    string(REPLACE "share/crashpad" "tools/crashpad" _crashpad_tools
                   "${crashpad_DIR}")
  endif()

  if(NOT EXISTS "${_crashpad_tools}")
    message(FATAL_ERROR "Crashpad tools directory not found: ${_crashpad_tools}")
  endif()

  set(${output_var} "${_crashpad_tools}")
  message(STATUS "Crashpad tools directory: ${${output_var}}")
endmacro()

# -----------------------------------------------------------------------------
# 构建后把 handler 同步到运行时目录
#   用法：setup_crashpad_handler(<target> <target_dir>)
# -----------------------------------------------------------------------------
function(setup_crashpad_handler target_name target_dir)
  if(NOT TARGET ${target_name})
    message(FATAL_ERROR "setup_crashpad_handler: 目标不存在: ${target_name}")
  endif()

  get_crashpad_tools_path(_crashpad_tools)

  # CMake 3.26 起有 copy_directory_if_different，旧版本退化为 copy_directory
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.26)
    set(_copy_cmd ${CMAKE_COMMAND} -E copy_directory_if_different)
  else()
    set(_copy_cmd ${CMAKE_COMMAND} -E copy_directory)
  endif()

  add_custom_command(
    TARGET ${target_name}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${target_dir}"
    COMMAND ${_copy_cmd} "${_crashpad_tools}" "${target_dir}"
    COMMENT "Syncing crashpad handler to ${target_dir}"
    VERBATIM)
endfunction()

# -----------------------------------------------------------------------------
# 安装期拷贝 handler（可执行权限）
# -----------------------------------------------------------------------------
function(install_crashpad_handler destination)
  get_crashpad_tools_path(_crashpad_tools)

  install(
    DIRECTORY "${_crashpad_tools}/"
    DESTINATION "${destination}"
    FILE_PERMISSIONS
      OWNER_READ
      OWNER_WRITE
      GROUP_READ
      WORLD_READ
    DIRECTORY_PERMISSIONS
      OWNER_READ
      OWNER_WRITE
      OWNER_EXECUTE
      GROUP_READ
      GROUP_EXECUTE
      WORLD_READ
      WORLD_EXECUTE
    PATTERN "crashpad_handler"
            PERMISSIONS
            OWNER_READ
            OWNER_WRITE
            OWNER_EXECUTE
            GROUP_READ
            GROUP_EXECUTE
            WORLD_READ
            WORLD_EXECUTE)
endfunction()
