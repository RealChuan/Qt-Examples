# cmake-format: off
# Target Architecture Detection
#
# 对外统一输出：x86_64 / arm64 / x86 / universal / unknown
# （目录名 x64 由 OutputDirectories.cmake 负责映射，这里不做改名）
#
# 优先级（全部来自环境，不接受外部指定）：
#   1. APPLE 下的 CMAKE_OSX_ARCHITECTURES（含通用二进制）
#   2. CMAKE_GENERATOR_PLATFORM（VS 的 -A 参数）
#   3. MSVC + Ninja 时的编译器路径（.../Hostx64/arm64/cl.exe）
#   4. CMAKE_CROSSCOMPILING 时的 CMAKE_SYSTEM_PROCESSOR
#   5. CMAKE_HOST_SYSTEM_PROCESSOR（本机）
#   6. uname -m 兜底
#
# 平台判断用 APPLE / WIN32：本文件在 project() 之后运行，拿的是目标平台。
# 在 project() 之前运行的文件（VcpkgToolchain.cmake）只能用 CMAKE_HOST_*。
# cmake-format: on

# 把各种写法归一成内部命名，无法识别时返回空串
function(normalize_architecture value out_var)
  string(TOLOWER "${value}" _raw)
  if(_raw MATCHES "^(x64|x86_64|amd64)$")
    set(_result "x86_64")
  elseif(_raw MATCHES "^(arm64|aarch64)$")
    set(_result "arm64")
  elseif(_raw MATCHES "^(x86|i[3-6]86|win32)$")
    set(_result "x86")
  elseif(_raw STREQUAL "universal")
    set(_result "universal")
  else()
    set(_result "")
  endif()
  set(${out_var}
      "${_result}"
      PARENT_SCOPE)
endfunction()

# 在没有显式值的情况下推断目标架构
function(detect_architecture out_var)
  set(_result "")

  # macOS 通用二进制 / 指定架构（仅 Apple 有效，其它平台的同名变量必须忽略）
  if(APPLE AND CMAKE_OSX_ARCHITECTURES)
    list(FIND CMAKE_OSX_ARCHITECTURES "arm64" _has_arm64)
    list(FIND CMAKE_OSX_ARCHITECTURES "x86_64" _has_x86_64)
    if(_has_arm64 GREATER_EQUAL 0 AND _has_x86_64 GREATER_EQUAL 0)
      set(_result "universal")
    elseif(_has_arm64 GREATER_EQUAL 0)
      set(_result "arm64")
    elseif(_has_x86_64 GREATER_EQUAL 0)
      set(_result "x86_64")
    endif()
  endif()

  # VS 生成器的平台参数
  if(NOT _result AND CMAKE_GENERATOR_PLATFORM)
    normalize_architecture("${CMAKE_GENERATOR_PLATFORM}" _result)
  endif()

  # MSVC + Ninja：编译器路径本身就带着目标架构（.../Hostx64/arm64/cl.exe）
  if(NOT _result
     AND MSVC
     AND CMAKE_CXX_COMPILER)
    if(CMAKE_CXX_COMPILER MATCHES "[/\\\\](arm64|aarch64)[/\\\\]cl\\.exe$")
      set(_result "arm64")
    elseif(CMAKE_CXX_COMPILER MATCHES "[/\\\\]x86[/\\\\]cl\\.exe$")
      set(_result "x86")
    elseif(CMAKE_CXX_COMPILER MATCHES "[/\\\\](x64|amd64)[/\\\\]cl\\.exe$")
      set(_result "x86_64")
    endif()
  endif()

  # cmake-format: off
  # 只有交叉编译时 CMAKE_SYSTEM_PROCESSOR 才代表目标。
  #   不能无条件使用：macOS 上它来自 uname -p，Apple Silicon 给 "arm"、Intel 给
  #   "i386"，后者会被归一成 32 位 x86，把整个产物目录与 vcpkg triplet 带偏。
  # cmake-format: on
  if(NOT _result AND CMAKE_CROSSCOMPILING)
    normalize_architecture("${CMAKE_SYSTEM_PROCESSOR}" _result)
  endif()

  if(NOT _result)
    normalize_architecture("${CMAKE_HOST_SYSTEM_PROCESSOR}" _result)
  endif()

  # 兜底：宿主机架构也给不出可识别值时，直接问内核
  if(NOT _result AND NOT WIN32)
    execute_process(
      COMMAND uname -m
      OUTPUT_VARIABLE _uname_machine
      OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    normalize_architecture("${_uname_machine}" _result)
  endif()

  if(NOT _result)
    message(
      WARNING "无法可靠推断目标架构：请检查 CMAKE_OSX_ARCHITECTURES / CMAKE_CROSSCOMPILING，"
              "或改用显式描述目标架构的工具链文件")
    set(_result "unknown")
  endif()

  set(${out_var}
      "${_result}"
      PARENT_SCOPE)
endfunction()

detect_architecture(_arch_detected)
set(TARGET_ARCH
    "${_arch_detected}"
    CACHE STRING "Target architecture (x86_64/arm64/x86/universal)" FORCE)
message(STATUS "Target architecture (detected): ${TARGET_ARCH}")
