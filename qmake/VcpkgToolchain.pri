# VcpkgToolchain.pri - vcpkg 依赖配置

# 包含架构检测
include(ArchitectureDetection.pri)

# 设置平台特定的 vcpkg 路径
win32 {
    vcpkg_path = C:/vcpkg/installed/$$TARGET_ARCH-windows-static-md
}

macx {
    vcpkg_path = /usr/local/share/vcpkg/installed/$$TARGET_ARCH-osx
}

unix:!macx {
    vcpkg_path = /usr/local/share/vcpkg/installed/$$TARGET_ARCH-linux
}

# 输出 vcpkg 路径信息
message("Using vcpkg path: $$vcpkg_path")

# 设置调试后缀和库路径
CONFIG(debug, debug|release) {
    suffix = d
    LIBS += -L$$vcpkg_path/debug/lib
} else {
    suffix =
    LIBS += -L$$vcpkg_path/lib
}

# Breakpad 库
LIBS += -llibbreakpad_client$$suffix -llibbreakpad$$suffix

# Crashpad 库
LIBS += -lvcpkg_crashpad_client_common -lvcpkg_crashpad_client -lvcpkg_crashpad_util -lvcpkg_crashpad_base

# 包含路径
INCLUDEPATH += \
    $$vcpkg_path/include \
    $$vcpkg_path/include/crashpad

# for crashpad
win32 {
    LIBS += -lzlib$$suffix -lAdvapi32
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
