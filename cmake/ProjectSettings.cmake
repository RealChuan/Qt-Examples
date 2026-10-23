# =============================================================================
# Standard Project Settings for Modern C++ Projects
#   注意：TARGET_ARCH 由 ArchitectureDetection.cmake 统一归一化后写入缓存，
#   这里不再声明空缓存项，避免「空值 + PARENT_SCOPE」两套写法并存。
# =============================================================================

include(cmake/CompilerSettings.cmake)
include(cmake/ArchitectureDetection.cmake)
include(cmake/BuildConfigurations.cmake)
include(cmake/OutputDirectories.cmake)

# 交叉编译设置
if(CMAKE_CROSSCOMPILING)
  message(
    STATUS
      "Cross-compiling: ${CMAKE_HOST_SYSTEM_PROCESSOR} -> ${CMAKE_SYSTEM_PROCESSOR}"
  )

  # 交叉编译时只允许在 root path 下查找库/头文件/包
  if(TARGET_ARCH STREQUAL "arm64" AND CMAKE_FIND_ROOT_PATH)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
  endif()
endif()
