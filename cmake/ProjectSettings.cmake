# =============================================================================
# Standard Project Settings for Modern C++ Projects
# =============================================================================

set(TARGET_ARCH
    ""
    CACHE STRING "Target architecture (x86_64, arm64, universal, etc.)")

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

  # 设置交叉编译查找规则
  if(TARGET_ARCH STREQUAL "arm64")
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
  endif()
endif()
