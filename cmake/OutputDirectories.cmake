# =============================================================================
# Output Directory Configuration
# =============================================================================

# 确定架构目录名
if(TARGET_ARCH STREQUAL "x86_64")
  set(ARCH_DIR "x64")
elseif(TARGET_ARCH STREQUAL "arm64")
  set(ARCH_DIR "arm64")
elseif(TARGET_ARCH STREQUAL "universal")
  set(ARCH_DIR "universal")
elseif(TARGET_ARCH STREQUAL "x86")
  set(ARCH_DIR "x86")
else()
  set(ARCH_DIR "unknown")
endif()

function(setup_output_directories)
  # 基础输出路径
  set(BASE_OUTPUT_DIR "${PROJECT_SOURCE_DIR}/binaries/${ARCH_DIR}")

  if(CMAKE_CONFIGURATION_TYPES)
    # 多配置生成器（Visual Studio, Xcode）
    foreach(config IN LISTS CMAKE_CONFIGURATION_TYPES)
      string(TOUPPER "${config}" CONFIG_UPPER)

      set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER}
          "${BASE_OUTPUT_DIR}/lib/${config}"
          PARENT_SCOPE)
      set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER}
          "${BASE_OUTPUT_DIR}/lib/${config}"
          PARENT_SCOPE)
      set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER}
          "${BASE_OUTPUT_DIR}/bin/${config}"
          PARENT_SCOPE)
    endforeach()

    # 设置默认目录
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
        "${BASE_OUTPUT_DIR}/lib"
        PARENT_SCOPE)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
        "${BASE_OUTPUT_DIR}/lib"
        PARENT_SCOPE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
        "${BASE_OUTPUT_DIR}/bin"
        PARENT_SCOPE)

  else()
    # 单配置生成器（Makefile, Ninja）
    if(CMAKE_BUILD_TYPE STREQUAL "None")
      set(OUTPUT_SUFFIX "none")
    else()
      string(TOLOWER "${CMAKE_BUILD_TYPE}" OUTPUT_SUFFIX)
    endif()

    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
        "${BASE_OUTPUT_DIR}/lib/${OUTPUT_SUFFIX}"
        PARENT_SCOPE)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
        "${BASE_OUTPUT_DIR}/lib/${OUTPUT_SUFFIX}"
        PARENT_SCOPE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
        "${BASE_OUTPUT_DIR}/bin/${OUTPUT_SUFFIX}"
        PARENT_SCOPE)
  endif()

  # 创建目录
  file(MAKE_DIRECTORY "${BASE_OUTPUT_DIR}/bin")
  file(MAKE_DIRECTORY "${BASE_OUTPUT_DIR}/lib")
endfunction()

# 便捷函数：为 target 设置库搜索路径（target 级替代 link_directories）
function(target_add_output_link_dirs target_name)
  if(CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    target_link_directories(${target_name} PRIVATE
      "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}"
    )
  endif()

  if(CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    target_link_directories(${target_name} PRIVATE
      "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
    )
  endif()
endfunction()

setup_output_directories()
