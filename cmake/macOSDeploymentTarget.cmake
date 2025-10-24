# =============================================================================
# macOS Deployment Target Configuration macOS部署目标配置
# =============================================================================

if(CMAKE_HOST_APPLE AND NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
  set(CMAKE_OSX_DEPLOYMENT_TARGET
      "13.0"
      CACHE STRING "Minimum OS X deployment version")
  message(
    STATUS "Set macOS deployment target to: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
endif()
