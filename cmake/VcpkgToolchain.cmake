# cmake-format: off
# Vcpkg Toolchain Configuration
#
# vcpkg 的安装位置在本项目里是固定的，直接硬编码，不做多候选探测：
#   - VS 安装器的 vcpkg 组件会把 VCPKG_ROOT 环境变量指向自带的空仓库
#     （%ProgramFiles%\Microsoft Visual Studio\<ver>\<edition>\VC\vcpkg），
#     跟随环境变量会走错目录并触发依赖全量重编
#   - 需要换位置时改 VCPKG_DEFAULT_ROOT 或 -DVCPKG_DEFAULT_ROOT=<dir>
#
# 注意：本文件在 project() 之前 include，WIN32/APPLE 尚不可用。
# cmake-format: on

option(USE_VCPKG "Use vcpkg for dependency management" ON)

if(NOT USE_VCPKG)
  message(STATUS "vcpkg: 已被 USE_VCPKG=OFF 关闭")
  return()
endif()

# 外部工具链（交叉编译等，非 vcpkg 自带）原样尊重
if(CMAKE_TOOLCHAIN_FILE AND NOT CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg\\.cmake$")
  message(STATUS "vcpkg: 使用外部工具链 ${CMAKE_TOOLCHAIN_FILE}，跳过 vcpkg 配置")
  return()
endif()

# -----------------------------------------------------------------------------
# 根目录（默认值即平台的常规安装位置，CACHE 只为方便在 cmake-gui 里改）
# -----------------------------------------------------------------------------
if(CMAKE_HOST_WIN32)
  set(_default_root "C:/vcpkg")
else()
  set(_default_root "/usr/local/share/vcpkg")
endif()

set(VCPKG_DEFAULT_ROOT
    "${_default_root}"
    CACHE PATH "vcpkg 根目录")

set(_vcpkg_toolchain "${VCPKG_DEFAULT_ROOT}/scripts/buildsystems/vcpkg.cmake")
if(NOT EXISTS "${_vcpkg_toolchain}")
  message(WARNING "vcpkg: 未找到 ${_vcpkg_toolchain}\n"
                  "  请安装 vcpkg，或用 -DVCPKG_DEFAULT_ROOT=<dir> 指定根目录")
  return()
endif()

# 强制写回缓存：本项目只认上面这一个位置，顺便覆盖历史配置里残留的旧路径
set(CMAKE_TOOLCHAIN_FILE
    "${_vcpkg_toolchain}"
    CACHE FILEPATH "Vcpkg toolchain file" FORCE)
message(STATUS "vcpkg: 工具链 ${CMAKE_TOOLCHAIN_FILE}")

# cmake-format: off
# triplet：只托管 Windows 的 ABI 选择（静态 vcpkg 库 + 动态 CRT，与 qmake 侧一致）。
# macOS / Linux 不指定，交给 vcpkg 自己按宿主架构识别；真正交叉编译时用
# -DVCPKG_TARGET_TRIPLET=<triplet> 显式指定。
#
# 本文件在 project() 之前 include，APPLE / WIN32 尚未定义，平台判断只能用
# CMAKE_HOST_*（与 macOSDeploymentTarget.cmake 同理）。
# cmake-format: on
if(CMAKE_HOST_WIN32 AND NOT VCPKG_TARGET_TRIPLET)
  # 与 vcpkg.cmake 同一思路：Ninja + MSVC 交叉编译时环境里没有目标架构， 只能看 cl.exe
  # 所在目录（.../Hostx64/arm64/cl.exe）。
  set(_triplet_arch "x64")
  find_program(_cl_path cl)
  if(_cl_path MATCHES "[/\\\\]arm64[/\\\\]cl\\.exe$")
    set(_triplet_arch "arm64")
  elseif(_cl_path MATCHES "[/\\\\]x86[/\\\\]cl\\.exe$")
    set(_triplet_arch "x86")
  endif()

  set(_triplet "${_triplet_arch}-windows-static-md")
  if(EXISTS "${VCPKG_DEFAULT_ROOT}/installed/${_triplet}")
    set(VCPKG_TARGET_TRIPLET
        "${_triplet}"
        CACHE STRING "vcpkg target triplet" FORCE)
    message(STATUS "vcpkg: triplet ${_triplet}（与 qmake 侧一致）")
  else()
    message(STATUS "vcpkg: 未安装 ${_triplet}，沿用 vcpkg 默认 triplet；"
                   "需与 qmake 侧一致时执行：vcpkg install --triplet ${_triplet}")
  endif()
endif()
