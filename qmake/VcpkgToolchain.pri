# =============================================================================
# VcpkgToolchain.pri - vcpkg 依赖配置
#
# vcpkg 的位置固定，直接硬编码，不读 VCPKG_ROOT 环境变量
# （VS 安装器会把它指向自带的空仓库，跟随它会走错目录）。
# triplet 用 ARCH_DIR（x64/arm64），与 cmake/VcpkgToolchain.cmake 保持一致。
# =============================================================================

include(ArchitectureDetection.pri)

win32 {
    vcpkg_path = C:/vcpkg/installed/$${ARCH_DIR}-windows-static-md
}

macx {
    vcpkg_path = /usr/local/share/vcpkg/installed/$${ARCH_DIR}-osx
}

unix:!macx {
    vcpkg_path = /usr/local/share/vcpkg/installed/$${ARCH_DIR}-linux
}

!exists($$vcpkg_path): warning("vcpkg: 未找到 $$vcpkg_path")

!isEmpty(VERBOSE): message("Using vcpkg path: $$vcpkg_path")

# 设置调试后缀和库路径
CONFIG(debug, debug|release) {
    suffix = d
    LIBS += -L$$vcpkg_path/debug/lib
} else {
    suffix =
    LIBS += -L$$vcpkg_path/lib
}

# Crashpad 库
LIBS += -lvcpkg_crashpad_client_common -lvcpkg_crashpad_client -lvcpkg_crashpad_util -lvcpkg_crashpad_base

# 包含路径
INCLUDEPATH += \
    $$vcpkg_path/include \
    $$vcpkg_path/include/crashpad

# for crashpad
win32 {
    LIBS += -lzs$$suffix -lAdvapi32
    DEFINES += NOMINMAX
}

unix {
    LIBS += -lz
}

macx {
    LIBS += \
        -lbsm \
        -lmig_output \
        -framework ApplicationServices \
        -framework CoreFoundation \
        -framework Foundation \
        -framework IOKit \
        -framework Security \
        -framework AppKit
}
