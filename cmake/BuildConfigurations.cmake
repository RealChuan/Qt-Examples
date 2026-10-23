# =============================================================================
# Build Configuration Settings
# =============================================================================

# 单配置生成器（Ninja / Makefile）必须显式给 CMAKE_BUILD_TYPE，否则默认无优化，
# 这里统一回退到 RelWithDebInfo（带符号，便于本地排查）。
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(
    STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
  set(CMAKE_BUILD_TYPE
      RelWithDebInfo
      CACHE STRING "Build type" FORCE)

  # 为cmake-gui提供可选值
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
                                               "MinSizeRel" "RelWithDebInfo")
endif()

# 多配置生成器（VS / Xcode）下 CMAKE_BUILD_TYPE 无效，构建期用 --config 选择
if(CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Multi-config generator: ${CMAKE_CONFIGURATION_TYPES}")
endif()
