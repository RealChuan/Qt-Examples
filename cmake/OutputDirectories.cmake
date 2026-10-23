# cmake-format: off
# Output Directory Configuration
#
#   TARGET_ARCH 由 ArchitectureDetection.cmake 统一检测产出（已归一化为
#   x86_64/arm64/x86/universal）。这里只做「目录映射」。
#
#   产物布局（packaging/ 与 CI 依赖，勿改）：
#     binaries/cmake/<ARCH_DIR>/bin/<config>/
#     binaries/cmake/<ARCH_DIR>/lib/<config>/
#
#   根目录下按构建系统分家（cmake/ 与 qmake/）：两条链的产物同名（都带 d 后缀），
#   合到一个目录会互相覆盖，隔离后也便于确认产物来自哪条链。
# cmake-format: on

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
  message(
    WARNING
      "OutputDirectories: 未知 TARGET_ARCH=${TARGET_ARCH}，产物将落到 binaries/unknown/"
  )
endif()

set(PROJECT_OUTPUT_ROOT "${PROJECT_SOURCE_DIR}/binaries/cmake/${ARCH_DIR}")

if(CMAKE_CONFIGURATION_TYPES)
  # 多配置生成器（Visual Studio, Xcode）：每个配置单独目录
  foreach(_config IN LISTS CMAKE_CONFIGURATION_TYPES)
    string(TOUPPER "${_config}" _config_upper)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${_config_upper}
        "${PROJECT_OUTPUT_ROOT}/lib/${_config}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_config_upper}
        "${PROJECT_OUTPUT_ROOT}/lib/${_config}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_config_upper}
        "${PROJECT_OUTPUT_ROOT}/bin/${_config}")
  endforeach()

  # 未指定 --config 时的默认目录
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_OUTPUT_ROOT}/lib")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_OUTPUT_ROOT}/lib")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_OUTPUT_ROOT}/bin")

  # 插件等子目录按配置追加 /<CONFIG>（由 set_target_output_dir 负责拼接）
  set(PROJECT_RUNTIME_BASE "${PROJECT_OUTPUT_ROOT}/bin")
  set(PROJECT_CONFIG_DIR "")
else()
  # 单配置生成器（Makefile, Ninja）
  if(CMAKE_BUILD_TYPE STREQUAL "None" OR NOT CMAKE_BUILD_TYPE)
    set(_output_suffix "none")
  else()
    string(TOLOWER "${CMAKE_BUILD_TYPE}" _output_suffix)
  endif()

  set(PROJECT_CONFIG_DIR "${_output_suffix}")

  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
      "${PROJECT_OUTPUT_ROOT}/lib/${_output_suffix}")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
      "${PROJECT_OUTPUT_ROOT}/lib/${_output_suffix}")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
      "${PROJECT_OUTPUT_ROOT}/bin/${_output_suffix}")

  # 单配置生成器下 CMAKE_RUNTIME_OUTPUT_DIRECTORY 已含配置后缀
  set(PROJECT_RUNTIME_BASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif()

file(MAKE_DIRECTORY "${PROJECT_OUTPUT_ROOT}/bin")
file(MAKE_DIRECTORY "${PROJECT_OUTPUT_ROOT}/lib")

message(
  STATUS
    "Output: ${PROJECT_OUTPUT_ROOT}/{bin,lib}/ (ARCH_DIR=${ARCH_DIR}, TARGET_ARCH=${TARGET_ARCH})"
)

# cmake-format: off
# 覆盖单个目标的运行时/库输出目录
#   - 多配置生成器下 RUNTIME_OUTPUT_DIRECTORY_<CONFIG> 在建目标时就已由全局值
#     初始化，只改不带后缀的属性不生效，必须逐个配置覆盖
#   - 这里显式拼接 /<CONFIG>，不用 $<CONFIG>：保证各配置目录与可执行文件
#     所在目录大小写一致（VS 生成器的 Debug 目录大小写与配置名并不一致）
# cmake-format: on
function(set_target_output_dir target_name output_base)
  set_target_properties(
    ${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${output_base}"
                              LIBRARY_OUTPUT_DIRECTORY "${output_base}")

  if(CMAKE_CONFIGURATION_TYPES)
    foreach(_cfg IN LISTS CMAKE_CONFIGURATION_TYPES)
      string(TOUPPER "${_cfg}" _CFG)
      set_target_properties(
        ${target_name}
        PROPERTIES "RUNTIME_OUTPUT_DIRECTORY_${_CFG}" "${output_base}/${_cfg}"
                   "LIBRARY_OUTPUT_DIRECTORY_${_CFG}" "${output_base}/${_cfg}")
    endforeach()
  endif()
endfunction()
