# =============================================================================
# 输出目录（与 CMake 的 binaries/<toolchain>/<arch>/... 布局对等）
#
#   binaries/cmake/... ← CMake
#   binaries/qmake/... ← qmake（本文件）
#
# 根目录下按构建系统分家：两条链产物同名（都带 d 后缀），合到一起会互相覆盖。
# =============================================================================

CONFIG(debug, debug|release) {
    BUILD_TYPE_DIR = debug
} else {
    BUILD_TYPE_DIR = release
}

OUTPUT_ROOT = $$PWD/../binaries/qmake/$$ARCH_DIR

RUNTIME_OUTPUT_DIRECTORY = $$OUTPUT_ROOT/bin/$$BUILD_TYPE_DIR
LIBRARY_OUTPUT_DIRECTORY = $$OUTPUT_ROOT/lib/$$BUILD_TYPE_DIR

!isEmpty(VERBOSE): message("RUNTIME_OUTPUT_DIRECTORY: $$RUNTIME_OUTPUT_DIRECTORY")

INCLUDEPATH += $$PWD/../src
DEPENDPATH  += $$PWD/../src
