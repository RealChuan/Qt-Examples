# =============================================================================
# Target Architecture Detection
# =============================================================================

function(detect_target_architecture)
  # 最高优先级: 检查macOS特定架构设置
  if(CMAKE_HOST_APPLE AND CMAKE_OSX_ARCHITECTURES)
    # 检查是否为通用二进制文件（包含多个架构）
    list(FIND CMAKE_OSX_ARCHITECTURES "x86_64" HAS_X86_64)
    list(FIND CMAKE_OSX_ARCHITECTURES "arm64" HAS_ARM64)

    # 如果同时包含x86_64和arm64，则为通用二进制文件
    if(HAS_X86_64 GREATER_EQUAL 0 AND HAS_ARM64 GREATER_EQUAL 0)
      set(TARGET_ARCH
          "universal"
          PARENT_SCOPE)
      message(STATUS "Target architecture: universal (x86_64 + arm64)")
      return()
    endif()

    # 单一架构处理
    list(GET CMAKE_OSX_ARCHITECTURES 0 DETECTED_ARCH)
    if(DETECTED_ARCH MATCHES "arm64")
      set(TARGET_ARCH
          "arm64"
          PARENT_SCOPE)
      message(
        STATUS "Target architecture (from CMAKE_OSX_ARCHITECTURES): arm64")
      return()
    elseif(DETECTED_ARCH MATCHES "x86_64")
      set(TARGET_ARCH
          "x86_64"
          PARENT_SCOPE)
      message(
        STATUS "Target architecture (from CMAKE_OSX_ARCHITECTURES): x86_64")
      return()
    endif()
  endif()

  # 次高优先级: 检查Windows Generator Platform (-A 参数)
  if(CMAKE_GENERATOR_PLATFORM)
    if(CMAKE_GENERATOR_PLATFORM MATCHES "x64|AMD64")
      set(TARGET_ARCH
          "x86_64"
          PARENT_SCOPE)
      message(
        STATUS "Target architecture (from CMAKE_GENERATOR_PLATFORM): x86_64")
      return()
    elseif(CMAKE_GENERATOR_PLATFORM MATCHES "ARM64")
      set(TARGET_ARCH
          "arm64"
          PARENT_SCOPE)
      message(
        STATUS "Target architecture (from CMAKE_GENERATOR_PLATFORM): arm64")
      return()
    elseif(CMAKE_GENERATOR_PLATFORM MATCHES "Win32")
      set(TARGET_ARCH
          "x86"
          PARENT_SCOPE) # 注意这里是x86
      message(STATUS "Target architecture (from CMAKE_GENERATOR_PLATFORM): x86")
      return()
    endif()
  endif()

  # 检查交叉编译环境 (CMAKE_SYSTEM_PROCESSOR)
  if(CMAKE_CROSSCOMPILING)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64|arm64")
      set(TARGET_ARCH
          "arm64"
          PARENT_SCOPE)
      message(
        STATUS
          "Target architecture (from CMAKE_SYSTEM_PROCESSOR, cross-compiling): arm64"
      )
      return()
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
      set(TARGET_ARCH
          "x86_64"
          PARENT_SCOPE)
      message(
        STATUS
          "Target architecture (from CMAKE_SYSTEM_PROCESSOR, cross-compiling): x86_64"
      )
      return()
    endif()
  endif()

  # 最低优先级: 回退到本地主机架构检测
  if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64|arm64")
    set(TARGET_ARCH
        "arm64"
        PARENT_SCOPE)
    message(STATUS "Target architecture (detected from host): arm64")
  elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    set(TARGET_ARCH
        "x86_64"
        PARENT_SCOPE)
    message(STATUS "Target architecture (detected from host): x86_64")
  else()
    set(TARGET_ARCH
        "unknown"
        PARENT_SCOPE)
    message(
      WARNING
        "Could not reliably detect the target architecture. Consider specifying it explicitly."
    )
  endif()
endfunction()

detect_target_architecture()

if(TARGET_ARCH STREQUAL "unknown")
  # 备用检测方法
  if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(TARGET_ARCH "x86_64")
  else()
    execute_process(
      COMMAND uname -m
      OUTPUT_VARIABLE UNAME_M
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(UNAME_M MATCHES "aarch64|arm64")
      set(TARGET_ARCH "arm64")
    elseif(UNAME_M MATCHES "x86_64|amd64")
      set(TARGET_ARCH "x86_64")
    else()
      message(WARNING "Cannot detect target architecture automatically")
      set(TARGET_ARCH "unknown")
    endif()
  endif()
endif()

message(STATUS "Target architecture: ${TARGET_ARCH}")
