# =============================================================================
# Compiler Settings and Checks
# =============================================================================

# C++标准设置
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_DEBUG_POSTFIX "d")

include(CheckCXXCompilerFlag)

# 编译器版本检查
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12.0)
    message(
      FATAL_ERROR
        "GCC 12.0+ required for C++23 std::expected support. Current: ${CMAKE_CXX_COMPILER_VERSION}"
    )
  endif()

elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 15.0)
    message(
      FATAL_ERROR
        "Clang 15.0+ required for C++23 std::expected support. Current: ${CMAKE_CXX_COMPILER_VERSION}"
    )
  endif()

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.34)
    message(
      FATAL_ERROR
        "MSVC 19.34+ (VS 17.4) required for C++23 std::expected support. Current: ${CMAKE_CXX_COMPILER_VERSION}"
    )
  endif()
endif()

# 项目级编译选项：所有目标自动继承
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES
                                           "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)

  # 安全检查（仅当编译器支持时添加）
  check_cxx_compiler_flag(-fstack-protector-strong HAS_STACK_PROTECTOR)
  if(HAS_STACK_PROTECTOR)
    add_compile_options(-fstack-protector-strong)
  endif()

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  add_compile_options(/W3 /permissive-)
endif()
