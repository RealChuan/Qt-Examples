# =============================================================================
# Vcpkg Toolchain Configuration
# =============================================================================

option(USE_VCPKG "Use vcpkg for dependency management" ON)

function(configure_vcpkg_toolchain)
  if(NOT USE_VCPKG)
    message(STATUS "Vcpkg is disabled by configuration")
    return()
  endif()

  # 优先检查是否已经从命令行传入了工具链文件
  if(DEFINED CMAKE_TOOLCHAIN_FILE)
    message(
      STATUS "Using toolchain file from command line: ${CMAKE_TOOLCHAIN_FILE}")
    return()
  endif()

  # 检查缓存中是否已经设置了工具链文件
  get_property(
    TOOLCHAIN_FILE_IN_CACHE
    CACHE CMAKE_TOOLCHAIN_FILE
    PROPERTY VALUE)
  if(TOOLCHAIN_FILE_IN_CACHE)
    message(
      STATUS "Using toolchain file from cache: ${TOOLCHAIN_FILE_IN_CACHE}")
    return()
  endif()

  # 平台特定的vcpkg路径
  if(CMAKE_HOST_WIN32)
    set(VCPKG_ROOT "C:/vcpkg")
  else()
    set(VCPKG_ROOT "/usr/local/share/vcpkg")
  endif()
  set(VCPKG_DEFAULT_PATH "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

  # 检查工具链文件是否存在
  if(EXISTS "${VCPKG_DEFAULT_PATH}")
    message(STATUS "Using vcpkg toolchain file: ${VCPKG_DEFAULT_PATH}")
    set(CMAKE_TOOLCHAIN_FILE
        "${VCPKG_DEFAULT_PATH}"
        CACHE STRING "Vcpkg toolchain file")
  else()
    message(WARNING "Vcpkg toolchain file not found: ${VCPKG_DEFAULT_PATH}")
    message(STATUS "Please install vcpkg or set CMAKE_TOOLCHAIN_FILE manually")
  endif()
endfunction()

configure_vcpkg_toolchain()

if(DEFINED CMAKE_TOOLCHAIN_FILE)
  get_filename_component(VCPKG_ROOT "${CMAKE_TOOLCHAIN_FILE}/../../.." ABSOLUTE)
  message(STATUS "Vcpkg root: ${VCPKG_ROOT}")
endif()
