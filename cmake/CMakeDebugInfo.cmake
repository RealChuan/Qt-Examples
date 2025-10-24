# =============================================================================
# CMake Debug Information Utility
# =============================================================================

# -----------------------------------------------------------------------------
# 配置选项
# -----------------------------------------------------------------------------
option(PROJECT_DEBUG_INFO "Print detailed CMake configuration information" ON)

# -----------------------------------------------------------------------------
# 通用工具函数
# -----------------------------------------------------------------------------
function(get_current_timestamp output_var)
  string(TIMESTAMP timestamp "%Y-%m-%d %H:%M:%S")
  set(${output_var}
      ${timestamp}
      PARENT_SCOPE)
endfunction()

function(get_system_bitness output_var)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(${output_var}
        "64-bit"
        PARENT_SCOPE)
  else()
    set(${output_var}
        "32-bit"
        PARENT_SCOPE)
  endif()
endfunction()

# -----------------------------------------------------------------------------
# 系统信息显示函数
# -----------------------------------------------------------------------------
function(show_system_info)
  get_current_timestamp(CURRENT_TIME)
  message(STATUS "│ Timestamp       : ${CURRENT_TIME}")

  message(STATUS "│ CMake Version   : ${CMAKE_VERSION}")
  message(STATUS "│ Generator       : ${CMAKE_GENERATOR}")

  # 检测生成器额外信息
  if(CMAKE_GENERATOR MATCHES "Visual Studio")
    message(STATUS "│ Generator Platform: ${CMAKE_GENERATOR_PLATFORM}")
    if(CMAKE_GENERATOR_TOOLSET)
      message(STATUS "│ Generator Toolset : ${CMAKE_GENERATOR_TOOLSET}")
    endif()
  endif()

  message(
    STATUS "│ System          : ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION}")
  message(STATUS "│ Architecture    : ${CMAKE_SYSTEM_PROCESSOR}")

  get_system_bitness(BITNESS)
  message(STATUS "│ Bitness         : ${BITNESS}")

  # 检测是否为交叉编译
  if(CMAKE_CROSSCOMPILING)
    message(
      STATUS
        "│ Cross-compiling : YES (${CMAKE_HOST_SYSTEM_PROCESSOR} → ${CMAKE_SYSTEM_PROCESSOR})"
    )
  else()
    message(STATUS "│ Cross-compiling : NO")
  endif()
endfunction()

# -----------------------------------------------------------------------------
# 编译器信息显示函数
# -----------------------------------------------------------------------------
function(show_compiler_info)
  message(STATUS "│ C Compiler      : ${CMAKE_C_COMPILER}")
  message(
    STATUS
      "│ C Compiler ID   : ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
  message(STATUS "│ CXX Compiler    : ${CMAKE_CXX_COMPILER}")
  message(
    STATUS
      "│ CXX Compiler ID : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}"
  )
  message(
    STATUS
      "│ CXX Standard    : ${CMAKE_CXX_STANDARD} (Required: ${CMAKE_CXX_STANDARD_REQUIRED})"
  )
  message(STATUS "│ CXX Extensions  : ${CMAKE_CXX_EXTENSIONS}")
endfunction()

# -----------------------------------------------------------------------------
# 构建配置信息显示函数
# -----------------------------------------------------------------------------
function(show_build_config_info)
  message(STATUS "│ Build Type      : ${CMAKE_BUILD_TYPE}")

  # 多配置检测
  if(CMAKE_CONFIGURATION_TYPES)
    message(STATUS "│ Config Types    : ${CMAKE_CONFIGURATION_TYPES}")
  endif()

  message(STATUS "│ C Flags         : ${CMAKE_C_FLAGS}")
  message(STATUS "│ CXX Flags       : ${CMAKE_CXX_FLAGS}")

  if(CMAKE_EXE_LINKER_FLAGS)
    message(STATUS "│ Exe Linker Flags: ${CMAKE_EXE_LINKER_FLAGS}")
  endif()

  if(CMAKE_SHARED_LINKER_FLAGS)
    message(STATUS "│ Shared Linker   : ${CMAKE_SHARED_LINKER_FLAGS}")
  endif()

  if(CMAKE_STATIC_LINKER_FLAGS)
    message(STATUS "│ Static Linker   : ${CMAKE_STATIC_LINKER_FLAGS}")
  endif()
endfunction()

# -----------------------------------------------------------------------------
# 目录布局信息显示函数
# -----------------------------------------------------------------------------
function(show_directory_layout_info)
  message(STATUS "│ Source Directory: ${CMAKE_SOURCE_DIR}")
  message(STATUS "│ Binary Directory: ${CMAKE_BINARY_DIR}")

  # 显示项目特定输出目录
  if(DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    message(STATUS "│ Runtime Output  : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
  endif()
  if(DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    message(STATUS "│ Library Output  : ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
  endif()
  if(DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    message(STATUS "│ Archive Output  : ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
  endif()
endfunction()

# -----------------------------------------------------------------------------
# 安装配置信息显示函数
# -----------------------------------------------------------------------------
function(show_install_config_info)
  if(NOT INSTALL_CONFIG_LOADED)
    message(STATUS "│ Install Config   : Not loaded")
    return()
  endif()

  message(STATUS "│ Install Prefix   : ${CMAKE_INSTALL_PREFIX}")
  message(STATUS "│ Main App Dir     : ${MAIN_APP_INSTALL_DIR}")
  message(STATUS "│ Tool Dir         : ${TOOL_INSTALL_DIR}")
  message(STATUS "│ Library Dir      : ${LIB_INSTALL_DIR}")
  message(STATUS "│ Plugin Dir       : ${PLUGIN_INSTALL_DIR}")
  message(STATUS "│ Resource Dir     : ${RESOURCE_INSTALL_DIR}")
  message(STATUS "│ Translation Dir  : ${TRANSLATION_INSTALL_DIR}")

  # 显示平台特定的额外信息
  if(APPLE)
    message(STATUS "│ macOS Bundle     : ${MACOSX_BUNDLE_BUNDLE_NAME}.app")
    message(STATUS "│ Bundle ID        : ${MACOSX_BUNDLE_GUI_IDENTIFIER}")
    message(STATUS "│ Bundle Version   : ${MACOSX_BUNDLE_SHORT_VERSION_STRING}")
  endif()
endfunction()

# -----------------------------------------------------------------------------
# 特性信息显示函数
# -----------------------------------------------------------------------------
function(show_features_info)
  message(STATUS "│ Build Shared Libs: ${BUILD_SHARED_LIBS}")
  message(STATUS "│ Export Compile Cmds: ${CMAKE_EXPORT_COMPILE_COMMANDS}")

  # 检测其他常用特性
  if(DEFINED CMAKE_BUILD_RPATH)
    message(STATUS "│ Build RPATH     : ${CMAKE_BUILD_RPATH}")
  endif()
  if(DEFINED CMAKE_INSTALL_RPATH)
    message(STATUS "│ Install RPATH   : ${CMAKE_INSTALL_RPATH}")
  endif()

  # 检测编译特性支持
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag(-fstack-protector-strong HAS_STACK_PROTECTOR)
    message(STATUS "│ Stack Protector : ${HAS_STACK_PROTECTOR}")
  endif()
endfunction()

# -----------------------------------------------------------------------------
# 项目特定信息显示函数
# -----------------------------------------------------------------------------
function(show_project_specific_info)
  # 显示项目特定变量
  if(DEFINED PROJECT_NAME)
    message(STATUS "│ Project Name    : ${PROJECT_NAME}")
  endif()

  if(DEFINED PROJECT_VERSION)
    message(STATUS "│ Project Version : ${PROJECT_VERSION}")
  endif()

  if(DEFINED TARGET_ARCH)
    message(STATUS "│ Target Arch     : ${TARGET_ARCH}")
  endif()

  # 显示自定义缓存变量
  get_cmake_property(CACHE_VARS CACHE_VARIABLES)
  set(CUSTOM_CACHE_VARS "")
  foreach(CACHE_VAR ${CACHE_VARS})
    if(CACHE_VAR MATCHES "^PROJECT_"
       OR CACHE_VAR MATCHES "^CMAKE_"
       OR CACHE_VAR MATCHES "^BUILD_")
      get_property(
        CACHE_VAR_HELPSTRING
        CACHE ${CACHE_VAR}
        PROPERTY HELPSTRING)
      if(CACHE_VAR_HELPSTRING AND NOT CACHE_VAR_HELPSTRING STREQUAL "")
        list(APPEND CUSTOM_CACHE_VARS "${CACHE_VAR}=${${CACHE_VAR}}")
      endif()
    endif()
  endforeach()

  if(CUSTOM_CACHE_VARS)
    list(SORT CUSTOM_CACHE_VARS)
    message(STATUS "│ Custom Cache Vars:")
    foreach(VAR ${CUSTOM_CACHE_VARS})
      message(STATUS "│   ${VAR}")
    endforeach()
  endif()
endfunction()

# -----------------------------------------------------------------------------
# 主调试信息显示函数
# -----------------------------------------------------------------------------
function(show_cmake_debug_info)
  if(NOT PROJECT_DEBUG_INFO)
    return()
  endif()

  message(STATUS "")
  message(STATUS "┌───────────── CMake Configuration Debug Info ─────────────┐")

  # 系统信息
  message(STATUS "├─────────────────── System Information ───────────────────┤")
  show_system_info()

  # 编译器信息
  message(
    STATUS "├──────────────────── Compiler Information ─────────────────┤")
  show_compiler_info()

  # 构建配置
  message(
    STATUS "├───────────────────── Build Configuration ─────────────────┤")
  show_build_config_info()

  # 目录布局
  message(
    STATUS "├────────────────────── Directory Layout ───────────────────┤")
  show_directory_layout_info()

  # 安装配置
  message(
    STATUS "├───────────────────── Install Configuration ───────────────┤")
  show_install_config_info()

  # 特性信息
  message(
    STATUS "├───────────────────────── Features ────────────────────────┤")
  show_features_info()

  # 项目特定信息
  message(
    STATUS "├────────────────────── Project Specific ───────────────────┤")
  show_project_specific_info()

  message(
    STATUS "└────────────────────────────────────────────────────────────┘")
  message(STATUS "")
endfunction()

# -----------------------------------------------------------------------------
# 便捷函数：在特定条件下显示调试信息
# -----------------------------------------------------------------------------
function(show_debug_info_if_requested)
  # 如果用户明确要求或者设置了详细模式
  if(PROJECT_DEBUG_INFO OR CMAKE_VERBOSE_MAKEFILE)
    show_cmake_debug_info()
  endif()
endfunction()

# -----------------------------------------------------------------------------
# 单独显示某个信息块的函数（用于调试）
# -----------------------------------------------------------------------------
function(show_specific_debug_info)
  cmake_parse_arguments(
    ARG "SYSTEM;COMPILER;BUILD;DIRECTORY;INSTALL;FEATURES;PROJECT" "" ""
    ${ARGN})

  if(NOT PROJECT_DEBUG_INFO)
    return()
  endif()

  if(ARG_SYSTEM)
    message(STATUS "=== System Information ===")
    show_system_info()
  endif()

  if(ARG_COMPILER)
    message(STATUS "=== Compiler Information ===")
    show_compiler_info()
  endif()

  if(ARG_BUILD)
    message(STATUS "=== Build Configuration ===")
    show_build_config_info()
  endif()

  if(ARG_DIRECTORY)
    message(STATUS "=== Directory Layout ===")
    show_directory_layout_info()
  endif()

  if(ARG_INSTALL)
    message(STATUS "=== Install Configuration ===")
    show_install_config_info()
  endif()

  if(ARG_FEATURES)
    message(STATUS "=== Features ===")
    show_features_info()
  endif()

  if(ARG_PROJECT)
    message(STATUS "=== Project Specific ===")
    show_project_specific_info()
  endif()
endfunction()

# -----------------------------------------------------------------------------
# 测试函数（仅用于调试本模块）
# -----------------------------------------------------------------------------
function(test_debug_info_module)
  message(STATUS "Testing CMakeDebugInfo module...")
  set(PROJECT_DEBUG_INFO ON)

  # 测试完整信息
  message(STATUS "=== Testing Complete Debug Info ===")
  show_cmake_debug_info()

  # 测试单独模块
  message(STATUS "=== Testing Individual Modules ===")
  show_specific_debug_info(SYSTEM COMPILER)
endfunction()

# -----------------------------------------------------------------------------
# 自动注册调试信息显示（可选）
# -----------------------------------------------------------------------------
# 取消注释以下行以在包含此文件时自动显示调试信息（如果启用）
show_debug_info_if_requested()
