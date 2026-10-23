# cmake-format: off
# Library Target Utilities
#
# 统一负责三件事，避免每个子目录各写一份（两套构建系统由此保持同构）：
#   1. 平台决定库类型：Windows 动态库 / 其他平台静态库
#   2. 导出宏：<TARGET>_LIBRARY（动态）或 <TARGET>_STATIC_LIBRARY（静态）
#   3. 版本属性、安装规则
#
# Windows 下 DLL 由 CMAKE_RUNTIME_OUTPUT_DIRECTORY 直接落到 bin/<config>/，
# 导入库落到 lib/<config>/，不需要 POST_BUILD 复制。
# cmake-format: on

# -----------------------------------------------------------------------------
# 设置版本属性
# -----------------------------------------------------------------------------
function(set_target_version target_name)
  if(TARGET ${target_name} AND DEFINED PROJECT_VERSION)
    set_target_properties(
      ${target_name} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR}
                                VERSION ${PROJECT_VERSION})
  endif()
endfunction()

# cmake-format: off
# 写入导出宏
#   base 默认为目标名；thirdparty 这类历史拼写可用 EXPORT_MACRO 覆写
# cmake-format: on
function(_set_library_export_macro target_name base shared)
  string(TOUPPER "${base}" _macro_base)
  if(shared)
    target_compile_definitions(${target_name} PRIVATE "${_macro_base}_LIBRARY")
  else()
    target_compile_definitions(${target_name}
                               PRIVATE "${_macro_base}_STATIC_LIBRARY")
  endif()
endfunction()

# cmake-format: off
# 创建平台自适应库（Windows 动态库，其他平台静态库）
#   用法：add_platform_library(<name> [EXPORT_MACRO <base>] <sources>...)
# cmake-format: on
function(add_platform_library target_name)
  cmake_parse_arguments(ARG "" "EXPORT_MACRO" "" ${ARGN})
  set(_sources ${ARG_UNPARSED_ARGUMENTS})

  set(_macro_base "${target_name}")
  if(ARG_EXPORT_MACRO)
    set(_macro_base "${ARG_EXPORT_MACRO}")
  endif()

  if(WIN32)
    add_library(${target_name} SHARED ${_sources})
    _set_library_export_macro(${target_name} "${_macro_base}" TRUE)
    install(TARGETS ${target_name} RUNTIME DESTINATION ${LIB_INSTALL_DIR})
  else()
    add_library(${target_name} STATIC ${_sources})
    _set_library_export_macro(${target_name} "${_macro_base}" FALSE)
    # 静态库可能被链接进插件（动态库），GCC/Clang 需要 PIC
    set_target_properties(${target_name} PROPERTIES POSITION_INDEPENDENT_CODE
                                                    ON)
  endif()

  set_target_version(${target_name})
endfunction()

# -----------------------------------------------------------------------------
# 创建共享库（始终为动态库）
# -----------------------------------------------------------------------------
function(add_shared_library target_name)
  cmake_parse_arguments(ARG "" "EXPORT_MACRO" "" ${ARGN})
  set(_sources ${ARG_UNPARSED_ARGUMENTS})

  set(_macro_base "${target_name}")
  if(ARG_EXPORT_MACRO)
    set(_macro_base "${ARG_EXPORT_MACRO}")
  endif()

  add_library(${target_name} SHARED ${_sources})
  _set_library_export_macro(${target_name} "${_macro_base}" TRUE)

  set_target_version(${target_name})

  install(
    TARGETS ${target_name}
    RUNTIME DESTINATION ${LIB_INSTALL_DIR} # Windows: DLL
    LIBRARY DESTINATION ${LIB_INSTALL_DIR} # Linux: .so, macOS: .dylib
  )
endfunction()

# cmake-format: off
# 校验插件元数据
#   插件的 *.json 必须与目标名、项目版本一致，否则运行时容易出现
#   "Id 对不上 / 版本漂移"这类只能在界面上看出来的问题。
# cmake-format: on
function(validate_plugin_manifest target_name)
  set(_manifest "${CMAKE_CURRENT_SOURCE_DIR}/${target_name}.json")
  if(NOT EXISTS "${_manifest}")
    message(WARNING "插件 ${target_name} 缺少元数据文件: ${_manifest}")
    return()
  endif()

  file(READ "${_manifest}" _content)
  string(
    JSON
    _id
    ERROR_VARIABLE
    _id_error
    GET
    "${_content}"
    Id)
  if(_id_error)
    message(WARNING "插件 ${target_name} 的 JSON 里没有 Id 字段: ${_id_error}")
    return()
  endif()
  if(NOT _id STREQUAL "${target_name}")
    message(WARNING "插件 ${target_name} 的 Id (${_id}) 与目标名不一致，运行时按 Id 索引会找不到它")
  endif()

  string(
    JSON
    _version
    ERROR_VARIABLE
    _version_error
    GET
    "${_content}"
    Version)
  if(_version_error)
    message(WARNING "插件 ${target_name} 的 JSON 里没有 Version 字段")
  elseif(NOT _version STREQUAL PROJECT_VERSION)
    message(
      WARNING
        "插件 ${target_name} 的 Version (${_version}) 与 PROJECT_VERSION (${PROJECT_VERSION}) 不一致"
    )
  endif()
endfunction()

# cmake-format: off
# 创建插件库（跨平台，自动处理部署路径）
#   MODULE：只在运行期被 QPluginLoader 打开，不生成导入库、不允许被链接。
#   用法：add_app_plugin(<name> <sources>...)
# cmake-format: on
function(add_app_plugin target_name)
  add_library(${target_name} MODULE ${ARGN})
  _set_library_export_macro(${target_name} "${target_name}" TRUE)
  set_target_properties(${target_name} PROPERTIES PREFIX "" OUTPUT_NAME
                                                            "${target_name}")

  string(TOLOWER "${PROJECT_NAME}" _project_name_lowercase)

  # 平台差异集中在这里分叉，下面只走通用流程
  if(APPLE)
    # CMake 对 MODULE 的默认后缀是 .so，而 Qt 自身插件、qmake 的 CONFIG += plugin 与 macdeployqt
    # 的插件枚举（只认 *.dylib）都以 .dylib 为准。QPluginLoader
    # 两种都接受（QLibrary::isLibrary），这里对齐后缀， 免得按 *.dylib 过滤的部署/打包逻辑静默漏掉插件。
    set_target_properties(${target_name} PROPERTIES SUFFIX ".dylib")

    # 插件随 .app 一起分发：与主可执行文件同级
    set(_plugin_dest_dir
        "${PROJECT_RUNTIME_BASE}/${PROJECT_NAME}.app/Contents/PlugIns/${_project_name_lowercase}"
    )
  else()
    set(_plugin_dest_dir
        "${PROJECT_RUNTIME_BASE}/plugins/${_project_name_lowercase}")

    # 只有非 Apple 平台才加版本号：macOS 的 MODULE 以 -bundle 链接，clang 只允许 -dynamiclib 带
    # -current_version，而 VERSION 恰好会转成该标志； 插件不参与链接，本来也没有 install_name / ABI
    # 版本的概念。
    set_target_version(${target_name})

    if(UNIX)
      # 运行期要从 plugins/<project>/ 找到 ../../lib 里的依赖（Windows 无 RPATH）
      set_target_properties(${target_name} PROPERTIES INSTALL_RPATH
                                                      "$ORIGIN/../../lib")
    endif()
  endif()

  # 直接输出到插件目录，省掉每次构建的 POST_BUILD 复制
  set_target_output_dir(${target_name} "${_plugin_dest_dir}")

  install(
    TARGETS ${target_name}
    RUNTIME DESTINATION ${PLUGIN_INSTALL_DIR} # Windows: DLL
    LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR} # Linux: .so, macOS: .dylib
  )

  validate_plugin_manifest(${target_name})
endfunction()
