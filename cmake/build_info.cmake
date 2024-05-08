# 输出 CMake 版本和构建系统类型
message("CMake Version: ${CMAKE_VERSION}")
message("Generator: ${CMAKE_GENERATOR}")

# 输出编译器信息
message("C Compiler ID: ${CMAKE_C_COMPILER_ID}")
message("C++ Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message("C++ Compiler Version: ${CMAKE_CXX_COMPILER_VERSION}")

# 输出构建类型和编译选项
message("Build Type: ${CMAKE_BUILD_TYPE}")
message("C++ Compiler Flags: ${CMAKE_CXX_FLAGS}")

# 输出链接选项
message("Executable Linker Flags: ${CMAKE_EXE_LINKER_FLAGS}")

# 输出构建和源代码目录
message("Build Directory: ${CMAKE_BINARY_DIR}")
message("Source Directory: ${CMAKE_SOURCE_DIR}")

# 输出目标架构
message("Target Processor: ${CMAKE_SYSTEM_PROCESSOR}")
