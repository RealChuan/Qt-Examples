# cmake-format: off
# macOS Deployment Target Configuration macOS部署目标配置
#
# 本文件在 project() 之前 include：CMAKE_OSX_DEPLOYMENT_TARGET 必须早于编译器
# 探测才生效，VCPKG_OSX_DEPLOYMENT_TARGET 必须早于 vcpkg 端口构建。那时 APPLE
# 尚未定义，只能用 CMAKE_HOST_APPLE。
# cmake-format: on

if(CMAKE_HOST_APPLE AND NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
  if(DEFINED ENV{MACOSX_DEPLOYMENT_TARGET}
     AND NOT "$ENV{MACOSX_DEPLOYMENT_TARGET}" STREQUAL "")
    set(_deployment_target "$ENV{MACOSX_DEPLOYMENT_TARGET}")
  else()
    set(_deployment_target "14.4")
  endif()

  set(CMAKE_OSX_DEPLOYMENT_TARGET
      "${_deployment_target}"
      CACHE STRING "Minimum OS X deployment version")

  # cmake-format: off
  # vcpkg 的端口在 project() 期间就编译完毕，之后再设置对它们无效，所以必须在
  # 这里把同一个值交给它：CMake 端口由 vcpkg 写进 CMAKE_OSX_DEPLOYMENT_TARGET，
  # 非 CMake 端口（autotools/meson/gyp）由它拼成 -mmacosx-version-min。
  # 两处不同源时，链接阶段会报 "building for macOS 14.4 but linking with a file
  # built for macOS 15.0" 这类最低版本冲突。
  # cmake-format: on
  set(VCPKG_OSX_DEPLOYMENT_TARGET
      "${_deployment_target}"
      CACHE STRING "vcpkg 端口的最低 macOS 版本")
  set(ENV{MACOSX_DEPLOYMENT_TARGET} "${_deployment_target}")

  message(
    STATUS "Set macOS deployment target to: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
endif()
