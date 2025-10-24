# =============================================================================
# Library Target Utilities
# =============================================================================

# -----------------------------------------------------------------------------
# 设置版本属性
# -----------------------------------------------------------------------------
function(set_target_version target_name)
  if(DEFINED PROJECT_VERSION_MAJOR AND DEFINED PROJECT_VERSION)
    set_target_properties(
      ${target_name} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR}
                                VERSION ${PROJECT_VERSION})
  endif()
endfunction()

# -----------------------------------------------------------------------------
# 设置运行时输出目录函数（跨平台）
# -----------------------------------------------------------------------------
function(set_runtime_to_archive_dir target_name output_dir)
  # 设置默认输出目录
  set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                                  "${output_dir}")

  # 为所有构建配置设置输出目录
  foreach(OUTPUTCONFIG IN ITEMS DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    string(TOUPPER "${OUTPUTCONFIG}" UPPER_CONFIG)
    set_target_properties(
      ${target_name} PROPERTIES "RUNTIME_OUTPUT_DIRECTORY_${UPPER_CONFIG}"
                                "${output_dir}")
  endforeach()
endfunction()

# -----------------------------------------------------------------------------
# 创建平台自适应库（Windows动态库，其他平台静态库）
# -----------------------------------------------------------------------------
function(add_platform_library target_name)
  # 根据平台选择库类型
  if(CMAKE_HOST_WIN32)
    add_library(${target_name} SHARED ${ARGN})
    # Windows下需要特殊处理动态库输出目录
    set_runtime_to_archive_dir(${target_name}
                               "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")

    # 复制DLL到运行时目录
    add_custom_command(
      TARGET ${target_name}
      POST_BUILD
      COMMAND
        ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${target_name}>
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<TARGET_FILE_NAME:${target_name}>)

    install(TARGETS ${target_name} RUNTIME DESTINATION ${LIB_INSTALL_DIR})
  else()
    add_library(${target_name} STATIC ${ARGN})
  endif()

  set_target_version(${target_name})
endfunction()

# -----------------------------------------------------------------------------
# 创建插件库（跨平台，自动处理部署路径）
# -----------------------------------------------------------------------------
function(add_platform_plugin target_name)
  add_library(${target_name} SHARED ${ARGN})

  # Windows下需要特殊处理
  if(CMAKE_HOST_WIN32)
    set_runtime_to_archive_dir(${target_name}
                               "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
  endif()

  set_target_version(${target_name})

  string(TOLOWER "${PROJECT_NAME}" PROJECT_NAME_LOWERCASE)

  # 平台特定的复制路径
  if(CMAKE_HOST_APPLE)
    # macOS: 复制到 App Bundle 的 PlugIns 目录
    set(PLUGIN_DEST_DIR
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.app/Contents/PlugIns/${PROJECT_NAME_LOWERCASE}/"
    )
  else()
    set(PLUGIN_DEST_DIR
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins/${PROJECT_NAME_LOWERCASE}/")
  endif()

  # 复制插件到目标目录
  add_custom_command(
    TARGET ${target_name}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PLUGIN_DEST_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${target_name}>
            "${PLUGIN_DEST_DIR}/$<TARGET_FILE_NAME:${target_name}>")

  if(CMAKE_HOST_LINUX)
    set_target_properties(${target_name} PROPERTIES INSTALL_RPATH
                                                    "$ORIGIN/../../lib")
  endif()

  install(
    TARGETS ${target_name}
    RUNTIME DESTINATION ${PLUGIN_INSTALL_DIR} # Windows: DLL
    LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR} # Linux: .so, macOS: .dylib
  )
endfunction()

# -----------------------------------------------------------------------------
# 创建共享库（始终为动态库）
# -----------------------------------------------------------------------------
function(add_shared_library target_name)
  add_library(${target_name} SHARED ${ARGN})

  # Windows下需要特殊处理
  if(CMAKE_HOST_WIN32)
    set_runtime_to_archive_dir(${target_name}
                               "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
    add_custom_command(
      TARGET ${target_name}
      POST_BUILD
      COMMAND
        ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${target_name}>
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<TARGET_FILE_NAME:${target_name}>)
  endif()

  set_target_version(${target_name})

  install(
    TARGETS ${target_name}
    RUNTIME DESTINATION ${LIB_INSTALL_DIR} # Windows: DLL
    LIBRARY DESTINATION ${LIB_INSTALL_DIR} # Linux: .so, macOS: .dylib
  )
endfunction()
